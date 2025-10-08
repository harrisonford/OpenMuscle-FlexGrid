/**
 * OpenMuscle FlexGrid - Real-Time Gesture Recognition with BLE
 * 
 * This firmware performs real-time gesture recognition on the ESP32-S3
 * and broadcasts results via Bluetooth LE to connected devices.
 * 
 * Features:
 * - 60-channel pressure sensor reading
 * - IMU data integration
 * - On-device ML inference (TensorFlow Lite or simple classifier)
 * - BLE GATT server for data transmission
 * - OLED display for visual feedback
 * - Haptic feedback on gesture detection
 * 
 * Hardware: ESP32-S3 + OpenMuscle FlexGrid
 */

#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// ===== BLE UUIDs =====
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define GESTURE_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define COMMAND_CHAR_UUID   "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"
#define SENSOR_CHAR_UUID    "d1e2f3a4-5b6c-7d8e-9f0a-1b2c3d4e5f6a"

// ===== HARDWARE PINS =====
#define MUX_S0  32
#define MUX_S1  33
#define MUX_S2  25
#define MUX_S3  26
#define ROW_0   36
#define ROW_1   39
#define ROW_2   34
#define ROW_3   35
#define I2C_SDA 21
#define I2C_SCL 22
#define HAPTIC_PIN 27

// ===== DISPLAY CONFIGURATION =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== SENSOR CONFIGURATION =====
#define NUM_COLS 15
#define NUM_ROWS 4
#define TOTAL_SENSORS 60
#define SAMPLE_RATE_HZ 50
#define WINDOW_SIZE 50
#define CONFIDENCE_THRESHOLD 0.75

// ===== GLOBAL VARIABLES =====
BLEServer* pServer = NULL;
BLECharacteristic* pGestureCharacteristic = NULL;
BLECharacteristic* pCommandCharacteristic = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int sensorMatrix[NUM_ROWS][NUM_COLS];
float accel_x, accel_y, accel_z;
float gyro_x, gyro_y, gyro_z;

String currentGesture = "idle";
float gestureConfidence = 0.0;
unsigned long lastGestureTime = 0;
int gestureBuffer[WINDOW_SIZE][TOTAL_SENSORS + 6];  // Sensor + IMU data
int bufferIndex = 0;

// Gesture labels (update based on your trained model)
const char* GESTURE_LABELS[] = {
  "idle",
  "point_up",
  "point_down",
  "point_left",
  "point_right",
  "fist_close",
  "fist_open",
  "pinch",
  "swipe_left",
  "swipe_right",
  "twist_cw",
  "twist_ccw"
};
const int NUM_GESTURES = sizeof(GESTURE_LABELS) / sizeof(GESTURE_LABELS[0]);

// ===== BLE CALLBACKS =====
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client Connected");
    displayMessage("BLE", "Connected!", 1000);
    hapticFeedback(100, 1);
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client Disconnected");
    displayMessage("BLE", "Disconnected", 1000);
  }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.print("Received command: ");
      Serial.println(value.c_str());
      
      // Parse JSON command
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, value.c_str());
      
      if (!error) {
        const char* action = doc["action"];
        handleCommand(action, doc);
      }
    }
  }
};

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== OpenMuscle Gesture Recognition ===");
  Serial.println("Firmware v1.0 - BLE Edition\n");
  
  // Initialize hardware
  initializePins();
  initializeDisplay();
  initializeSensors();
  initializeBLE();
  
  Serial.println("✓ System ready");
  Serial.println("Waiting for BLE connection...\n");
  
  displayMessage("OpenMuscle", "Ready!", 2000);
}

// ===== MAIN LOOP =====
void loop() {
  // Handle BLE connection state
  if (deviceConnected != oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    
    if (!deviceConnected) {
      delay(500);  // Give time for BLE stack to prepare
      pServer->startAdvertising();  // Restart advertising
      Serial.println("Restarting BLE advertising...");
    }
  }
  
  // Read sensors and perform gesture recognition
  if (deviceConnected) {
    readAllSensors();
    readIMU();
    updateBuffer();
    
    // Perform gesture recognition every window
    if (bufferIndex >= WINDOW_SIZE) {
      String detectedGesture = recognizeGesture();
      
      if (detectedGesture != currentGesture && gestureConfidence > CONFIDENCE_THRESHOLD) {
        currentGesture = detectedGesture;
        lastGestureTime = millis();
        
        // Send gesture via BLE
        sendGesture(currentGesture, gestureConfidence);
        
        // Visual and haptic feedback
        displayGesture(currentGesture, gestureConfidence);
        hapticFeedback(50, 1);
        
        Serial.print("Gesture detected: ");
        Serial.print(currentGesture);
        Serial.print(" (confidence: ");
        Serial.print(gestureConfidence * 100);
        Serial.println("%)");
      }
      
      // Reset buffer with overlap
      shiftBuffer();
    }
  } else {
    // Display waiting message when not connected
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Waiting for");
    display.println("BLE connection...");
    display.display();
    delay(1000);
  }
  
  delay(20);  // 50Hz sampling
}

// ===== INITIALIZATION FUNCTIONS =====

void initializePins() {
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(ROW_0, INPUT);
  pinMode(ROW_1, INPUT);
  pinMode(ROW_2, INPUT);
  pinMode(ROW_3, INPUT);
  pinMode(HAPTIC_PIN, OUTPUT);
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void initializeDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERROR: SSD1306 allocation failed");
  } else {
    Serial.println("✓ OLED display initialized");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
  }
}

void initializeSensors() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);
  
  // Initialize IMU (add ICM42688 library initialization here)
  // Similar to data collector firmware
  
  Serial.println("✓ Sensors initialized");
}

void initializeBLE() {
  BLEDevice::init("OpenMuscle-FlexGrid");
  
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  
  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Gesture Characteristic (NOTIFY)
  pGestureCharacteristic = pService->createCharacteristic(
    GESTURE_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pGestureCharacteristic->addDescriptor(new BLE2902());
  
  // Command Characteristic (WRITE)
  pCommandCharacteristic = pService->createCharacteristic(
    COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCommandCharacteristic->setCallbacks(new CommandCallbacks());
  
  // Raw Sensor Characteristic (NOTIFY) - optional for debugging
  pSensorCharacteristic = pService->createCharacteristic(
    SENSOR_CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pSensorCharacteristic->addDescriptor(new BLE2902());
  
  // Start service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("✓ BLE server started");
  Serial.println("Device name: OpenMuscle-FlexGrid");
}

// ===== SENSOR READING =====

void readAllSensors() {
  for (int col = 0; col < NUM_COLS; col++) {
    selectMuxChannel(col);
    delayMicroseconds(10);
    
    sensorMatrix[0][col] = analogRead(ROW_0);
    sensorMatrix[1][col] = analogRead(ROW_1);
    sensorMatrix[2][col] = analogRead(ROW_2);
    sensorMatrix[3][col] = analogRead(ROW_3);
  }
}

void selectMuxChannel(int channel) {
  digitalWrite(MUX_S0, (channel & 0x01) ? HIGH : LOW);
  digitalWrite(MUX_S1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(MUX_S2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(MUX_S3, (channel & 0x08) ? HIGH : LOW);
}

void readIMU() {
  // Add IMU reading code here
  // For now, using dummy values
  accel_x = 0.0;
  accel_y = 0.0;
  accel_z = 1.0;
  gyro_x = 0.0;
  gyro_y = 0.0;
  gyro_z = 0.0;
}

void updateBuffer() {
  if (bufferIndex < WINDOW_SIZE) {
    // Flatten sensor matrix into buffer
    int idx = 0;
    for (int row = 0; row < NUM_ROWS; row++) {
      for (int col = 0; col < NUM_COLS; col++) {
        gestureBuffer[bufferIndex][idx++] = sensorMatrix[row][col];
      }
    }
    
    // Add IMU data
    gestureBuffer[bufferIndex][idx++] = (int)(accel_x * 1000);
    gestureBuffer[bufferIndex][idx++] = (int)(accel_y * 1000);
    gestureBuffer[bufferIndex][idx++] = (int)(accel_z * 1000);
    gestureBuffer[bufferIndex][idx++] = (int)(gyro_x * 1000);
    gestureBuffer[bufferIndex][idx++] = (int)(gyro_y * 1000);
    gestureBuffer[bufferIndex][idx++] = (int)(gyro_z * 1000);
    
    bufferIndex++;
  }
}

void shiftBuffer() {
  // Shift buffer by half window (50% overlap)
  int shift = WINDOW_SIZE / 2;
  for (int i = 0; i < shift; i++) {
    for (int j = 0; j < TOTAL_SENSORS + 6; j++) {
      gestureBuffer[i][j] = gestureBuffer[i + shift][j];
    }
  }
  bufferIndex = shift;
}

// ===== GESTURE RECOGNITION =====

String recognizeGesture() {
  /**
   * This is a placeholder for actual ML inference.
   * 
   * Option 1: Load TensorFlow Lite model and use ESP-NN for inference
   * Option 2: Implement simple rule-based classifier
   * Option 3: Use Random Forest converted to C code
   * 
   * For hackathon, you might start with simple heuristics:
   * - High pressure on certain sensors = fist
   * - IMU rotation = twist
   * - IMU tilt = point direction
   */
  
  // Simple heuristic example (replace with real ML inference)
  float avgPressure = calculateAveragePressure();
  float maxPressure = calculateMaxPressure();
  float pitchAngle = calculatePitch();
  
  gestureConfidence = 0.8;  // Placeholder
  
  // Rule-based classification
  if (maxPressure > 3000) {
    return "fist_close";
  } else if (pitchAngle > 45) {
    return "point_up";
  } else if (pitchAngle < -45) {
    return "point_down";
  } else if (avgPressure < 500) {
    return "fist_open";
  } else {
    return "idle";
  }
}

float calculateAveragePressure() {
  long sum = 0;
  int count = 0;
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      sum += sensorMatrix[row][col];
      count++;
    }
  }
  return (float)sum / count;
}

float calculateMaxPressure() {
  int maxVal = 0;
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      if (sensorMatrix[row][col] > maxVal) {
        maxVal = sensorMatrix[row][col];
      }
    }
  }
  return maxVal;
}

float calculatePitch() {
  // Calculate pitch angle from accelerometer
  return atan2(accel_y, sqrt(accel_x * accel_x + accel_z * accel_z)) * 180.0 / PI;
}

// ===== BLE COMMUNICATION =====

void sendGesture(String gesture, float confidence) {
  if (!deviceConnected) return;
  
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["gesture"] = gesture;
  doc["confidence"] = confidence;
  doc["timestamp"] = millis();
  doc["imu"]["pitch"] = calculatePitch();
  doc["imu"]["roll"] = calculateRoll();
  doc["imu"]["yaw"] = calculateYaw();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send via BLE
  pGestureCharacteristic->setValue(jsonString.c_str());
  pGestureCharacteristic->notify();
  
  Serial.println("Sent: " + jsonString);
}

float calculateRoll() {
  return atan2(-accel_x, accel_z) * 180.0 / PI;
}

float calculateYaw() {
  // Simplified - requires magnetometer for accurate yaw
  return 0.0;
}

void handleCommand(const char* action, JsonDocument& doc) {
  Serial.print("Executing action: ");
  Serial.println(action);
  
  if (strcmp(action, "calibrate") == 0) {
    displayMessage("Action", "Calibrating...", 1000);
    // Add calibration logic
  } else if (strcmp(action, "feedback") == 0) {
    int duration = doc["duration"] | 100;
    hapticFeedback(duration, 1);
  }
}

// ===== DISPLAY & FEEDBACK =====

void displayGesture(String gesture, float confidence) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("OpenMuscle");
  display.println("---------------");
  display.setTextSize(2);
  display.println(gesture);
  display.setTextSize(1);
  display.print("Conf: ");
  display.print((int)(confidence * 100));
  display.println("%");
  display.display();
}

void displayMessage(String title, String message, int duration) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(title);
  display.println("---------------");
  display.setTextSize(2);
  display.println(message);
  display.display();
  
  if (duration > 0) {
    delay(duration);
  }
}

void hapticFeedback(int duration_ms, int pulses) {
  for (int i = 0; i < pulses; i++) {
    digitalWrite(HAPTIC_PIN, HIGH);
    delay(duration_ms);
    digitalWrite(HAPTIC_PIN, LOW);
    if (i < pulses - 1) {
      delay(50);
    }
  }
}
