/**
 * OpenMuscle FlexGrid - Gesture Data Collector
 * 
 * This firmware collects raw sensor data from the 60-channel pressure sensor array
 * and IMU for machine learning training.
 * 
 * Hardware: ESP32-S3 with CD74HC4067 multiplexer and ICM-42688-P IMU
 * 
 * Usage:
 * 1. Upload firmware to ESP32-S3
 * 2. Open Serial Monitor at 115200 baud
 * 3. Send gesture label via serial (e.g., "point_up")
 * 4. Perform gesture 50-100 times
 * 5. Save serial output to CSV file
 */

#include <Wire.h>
#include <ICM42688.h>

// ===== HARDWARE PIN DEFINITIONS =====
// Multiplexer control pins
#define MUX_S0  32
#define MUX_S1  33
#define MUX_S2  25
#define MUX_S3  26

// ADC row select pins (connected to 4 different ADC channels)
#define ROW_0   36  // ADC1_CH0
#define ROW_1   39  // ADC1_CH3
#define ROW_2   34  // ADC1_CH6
#define ROW_3   35  // ADC1_CH7

// I2C pins for IMU and OLED
#define I2C_SDA 21
#define I2C_SCL 22

// ===== SENSOR CONFIGURATION =====
#define NUM_COLS 15  // CD74HC4067 has 16 channels, using 15
#define NUM_ROWS 4
#define TOTAL_SENSORS (NUM_COLS * NUM_ROWS)  // 60 sensors
#define SAMPLE_RATE_HZ 50  // 50Hz sampling
#define SAMPLE_INTERVAL_MS (1000 / SAMPLE_RATE_HZ)

// ===== GLOBAL VARIABLES =====
ICM42688 imu(Wire, 0x68);  // IMU I2C address
int sensorMatrix[NUM_ROWS][NUM_COLS];
float accel_x, accel_y, accel_z;
float gyro_x, gyro_y, gyro_z;
String currentGestureLabel = "idle";
bool isRecording = false;
unsigned long lastSampleTime = 0;
unsigned long sampleCount = 0;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== OpenMuscle Gesture Data Collector ===");
  Serial.println("Firmware v1.0\n");
  
  // Initialize multiplexer control pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  
  // Initialize ADC pins
  pinMode(ROW_0, INPUT);
  pinMode(ROW_1, INPUT);
  pinMode(ROW_2, INPUT);
  pinMode(ROW_3, INPUT);
  
  // Configure ADC resolution
  analogReadResolution(12);  // 12-bit ADC (0-4095)
  analogSetAttenuation(ADC_11db);  // Full range 0-3.3V
  
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);  // 400kHz I2C
  
  // Initialize IMU
  int status = imu.begin();
  if (status < 0) {
    Serial.println("ERROR: IMU initialization failed!");
    Serial.println("Check I2C connections and IMU address.");
  } else {
    Serial.println("✓ IMU initialized successfully");
    
    // Configure IMU settings
    imu.setAccelFS(ICM42688::gpm16);  // ±16g range
    imu.setGyroFS(ICM42688::dps2000); // ±2000 dps range
    imu.setAccelODR(ICM42688::odr100); // 100Hz
    imu.setGyroODR(ICM42688::odr100);  // 100Hz
  }
  
  Serial.println("✓ All sensors initialized");
  Serial.println("\n=== Data Collection Instructions ===");
  Serial.println("1. Send gesture label via serial (e.g., 'point_up')");
  Serial.println("2. Type 'START' to begin recording");
  Serial.println("3. Perform gesture repeatedly");
  Serial.println("4. Type 'STOP' to end recording");
  Serial.println("5. Save console output to CSV file\n");
  Serial.println("Available commands:");
  Serial.println("  - START: Begin recording");
  Serial.println("  - STOP: End recording");
  Serial.println("  - LABEL <name>: Set gesture label");
  Serial.println("  - TEST: Test sensor readings");
  Serial.println("  - HEADER: Print CSV header\n");
  
  printCSVHeader();
}

// ===== MAIN LOOP =====
void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    handleSerialCommand();
  }
  
  // Record data at specified sample rate
  if (isRecording && (millis() - lastSampleTime >= SAMPLE_INTERVAL_MS)) {
    lastSampleTime = millis();
    
    // Read all sensors
    readAllSensors();
    readIMU();
    
    // Output data as CSV row
    printDataRow();
    
    sampleCount++;
  }
  
  // Small delay to prevent CPU hogging
  delay(1);
}

// ===== SENSOR READING FUNCTIONS =====

/**
 * Read all 60 pressure sensors via multiplexer
 */
void readAllSensors() {
  for (int col = 0; col < NUM_COLS; col++) {
    // Set multiplexer channel
    selectMuxChannel(col);
    delayMicroseconds(10);  // Settling time
    
    // Read all 4 rows for this column
    sensorMatrix[0][col] = analogRead(ROW_0);
    sensorMatrix[1][col] = analogRead(ROW_1);
    sensorMatrix[2][col] = analogRead(ROW_2);
    sensorMatrix[3][col] = analogRead(ROW_3);
  }
}

/**
 * Select multiplexer channel (0-15)
 */
void selectMuxChannel(int channel) {
  digitalWrite(MUX_S0, (channel & 0x01) ? HIGH : LOW);
  digitalWrite(MUX_S1, (channel & 0x02) ? HIGH : LOW);
  digitalWrite(MUX_S2, (channel & 0x04) ? HIGH : LOW);
  digitalWrite(MUX_S3, (channel & 0x08) ? HIGH : LOW);
}

/**
 * Read IMU data (accelerometer and gyroscope)
 */
void readIMU() {
  imu.getAGT();  // Read accelerometer, gyroscope, temperature
  
  accel_x = imu.accX();
  accel_y = imu.accY();
  accel_z = imu.accZ();
  
  gyro_x = imu.gyrX();
  gyro_y = imu.gyrY();
  gyro_z = imu.gyrZ();
}

// ===== DATA OUTPUT FUNCTIONS =====

/**
 * Print CSV header
 */
void printCSVHeader() {
  Serial.print("timestamp,label,");
  
  // Pressure sensor columns (S0_0 to S3_14 = 60 sensors)
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      Serial.print("S");
      Serial.print(row);
      Serial.print("_");
      Serial.print(col);
      if (row < NUM_ROWS - 1 || col < NUM_COLS - 1) {
        Serial.print(",");
      }
    }
  }
  
  // IMU columns
  Serial.print(",accel_x,accel_y,accel_z");
  Serial.print(",gyro_x,gyro_y,gyro_z");
  Serial.println();
}

/**
 * Print data row in CSV format
 */
void printDataRow() {
  // Timestamp
  Serial.print(millis());
  Serial.print(",");
  
  // Label
  Serial.print(currentGestureLabel);
  Serial.print(",");
  
  // All 60 pressure sensors
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      Serial.print(sensorMatrix[row][col]);
      Serial.print(",");
    }
  }
  
  // IMU data
  Serial.print(accel_x, 4);
  Serial.print(",");
  Serial.print(accel_y, 4);
  Serial.print(",");
  Serial.print(accel_z, 4);
  Serial.print(",");
  Serial.print(gyro_x, 4);
  Serial.print(",");
  Serial.print(gyro_y, 4);
  Serial.print(",");
  Serial.print(gyro_z, 4);
  Serial.println();
}

// ===== COMMAND HANDLING =====

/**
 * Handle serial commands from user
 */
void handleSerialCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();
  
  if (command == "START") {
    isRecording = true;
    sampleCount = 0;
    lastSampleTime = millis();
    Serial.println("\n>>> RECORDING STARTED <<<");
    Serial.println("Perform gesture now...\n");
    
  } else if (command == "STOP") {
    isRecording = false;
    Serial.println("\n>>> RECORDING STOPPED <<<");
    Serial.print("Total samples collected: ");
    Serial.println(sampleCount);
    Serial.println();
    
  } else if (command.startsWith("LABEL ")) {
    currentGestureLabel = command.substring(6);
    currentGestureLabel.toLowerCase();
    currentGestureLabel.replace(" ", "_");
    Serial.print("Gesture label set to: ");
    Serial.println(currentGestureLabel);
    
  } else if (command == "TEST") {
    testSensors();
    
  } else if (command == "HEADER") {
    printCSVHeader();
    
  } else {
    Serial.println("Unknown command. Available commands:");
    Serial.println("  START, STOP, LABEL <name>, TEST, HEADER");
  }
}

/**
 * Test all sensors and display readings
 */
void testSensors() {
  Serial.println("\n=== Sensor Test ===");
  
  readAllSensors();
  readIMU();
  
  // Display pressure sensor matrix
  Serial.println("Pressure Sensors (60 channels):");
  for (int row = 0; row < NUM_ROWS; row++) {
    Serial.print("Row ");
    Serial.print(row);
    Serial.print(": ");
    for (int col = 0; col < NUM_COLS; col++) {
      Serial.print(sensorMatrix[row][col]);
      Serial.print("\t");
    }
    Serial.println();
  }
  
  // Display IMU data
  Serial.println("\nIMU Data:");
  Serial.print("Accel (g): X=");
  Serial.print(accel_x, 4);
  Serial.print(" Y=");
  Serial.print(accel_y, 4);
  Serial.print(" Z=");
  Serial.println(accel_z, 4);
  
  Serial.print("Gyro (dps): X=");
  Serial.print(gyro_x, 4);
  Serial.print(" Y=");
  Serial.print(gyro_y, 4);
  Serial.print(" Z=");
  Serial.println(gyro_z, 4);
  
  Serial.println("\n===================\n");
}
