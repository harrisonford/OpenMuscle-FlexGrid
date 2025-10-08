# 🧠 OpenMuscle Gesture Control System - Hackathon Plan
## Platanus Hackathon - Silent AI Interaction via Wristband

---

## 🎯 Project Vision

**Core Concept:** A wristband that enables silent, gesture-based control of devices and AI services, replacing voice commands with natural hand movements. Instead of saying "Hey AI, turn on the light," you simply point at the light and it responds.

**Key Differentiator:** Silent, intuitive interaction with AI and IoT devices through muscle activity detection rather than voice commands.

---

## 🔧 Hardware Overview - Your OpenMuscle FlexGrid

Based on your forked repository, you have access to the **OpenMuscle FlexGrid V1** with the following capabilities:

### Current Hardware Specifications:
- **MCU:** ESP32-S3-WROOM-1-N16R8
  - 16MB Flash / 8MB PSRAM
  - Built-in Wi-Fi & Bluetooth LE
  - Native USB support
  - Multiple ADC channels for sensor reading
  
- **Sensors:**
  - 60-channel pressure sensor matrix (15 cols × 4 rows via CD74HC4067 multiplexer)
  - ICM-42688-P IMU (6-DOF gyro + accelerometer)
  - Detects volumetric muscle contractions
  
- **Display:** SSD1306 OLED (I2C) for visual feedback

- **Power:**
  - LiPo battery charging via USB-C
  - TPS7A0333 LDO for 3.3V regulation
  - Smart pushbutton with MAX16054 controller
  
- **User Interface:**
  - Menu/Select buttons
  - Haptic feedback motor
  - Boot/Reset buttons

---

## 🧩 Components to Add for Full Functionality

### 1. **Bluetooth/Communication (Already Have It! ✓)**
Your ESP32-S3 has built-in BLE - no additional hardware needed.

### 2. **Audio Components (Optional but Recommended)**

**A. Microphone Module:**
- **Recommended:** INMP441 I2S Digital Microphone
  - I2S interface (ESP32 native support)
  - Low power consumption
  - Good for voice commands as fallback
  - ~$2-3 per unit
  
**B. Speaker/Audio Output:**
- **Option 1:** I2S DAC + Small Speaker
  - MAX98357A I2S Amplifier (~$5)
  - 3W 8Ω speaker
  - For audio feedback and "gibberlink" communication
  
- **Option 2:** Simple Buzzer (already might have via haptic motor)
  - Can generate simple tones
  - Cheaper but limited

**C. Alternative - Use existing haptic motor creatively:**
  - Different vibration patterns as feedback codes
  - Ultra-silent communication

### 3. **Additional Sensors (If Needed)**
- Your current setup should be sufficient
- The 60-sensor grid + IMU provides rich gesture data

---

## 🤖 AI/ML Architecture

### Phase 1: Gesture Recognition Model

**Recommended Approach: MuscleSenseNet + Custom Training**

1. **Base Model Options:**

   **A. MuscleSenseNet (OpenMuscle's Official Model)**
   - Repository: `openmuscle/muscle_sense_net` on HuggingFace
   - Uses Random Forest Regressor
   - Trained on forearm muscle pressure points
   - Predicts continuous finger movements
   - **Pro:** Already optimized for OpenMuscle hardware
   - **Con:** May need retraining for discrete gestures

   **B. TensorFlow Lite + CNN**
   - Build a Convolutional Neural Network
   - Input: 60-channel pressure data + IMU (6 axes)
   - Output: Gesture classification (8-15 discrete gestures)
   - Can be deployed on ESP32-S3
   - **Pro:** Real-time inference on device
   - **Con:** Requires more training data

   **C. Few-Shot Learning (FS-HGR Framework)**
   - Based on research: arxiv.org/abs/2011.06104
   - Requires minimal training data
   - Perfect for 36-hour timeframe
   - **Pro:** Quick adaptation to new users
   - **Con:** May have lower accuracy initially

2. **Gesture Set to Train (Recommended 8-12 gestures):**
   - 👉 Point (directional - up/down/left/right)
   - ✊ Fist close/open
   - ✌️ Two-finger swipe (left/right)
   - 🤏 Pinch
   - 🫰 Snap
   - 🤘 Rock gesture
   - 🖐️ Palm up/down
   - ↪️ Wrist rotation (CCW/CW)

3. **Training Pipeline:**

```
Data Collection (8-10 hours)
├── Record 60-channel pressure data + IMU
├── 50-100 samples per gesture per person
├── 3-5 test subjects if possible
└── Store as CSV/JSON with labels

Preprocessing (2-3 hours)
├── Normalize sensor values (0-1 range)
├── Apply sliding window (100-200ms)
├── Feature extraction (mean, std, FFT)
└── Data augmentation (noise, time-shift)

Model Training (4-6 hours)
├── Split: 80% train, 20% validation
├── Model architecture: CNN or Random Forest
├── Hyperparameter tuning
└── Target: >85% accuracy

Optimization for ESP32 (2-3 hours)
├── Convert to TensorFlow Lite
├── Quantization (int8 or float16)
├── Test inference speed (<100ms)
└── Deploy to ESP32-S3 flash
```

### Phase 2: MCP (Model Context Protocol) Integration

**What is MCP?**
- New protocol by Anthropic for AI tool integration
- Allows AI models to access external tools/APIs
- Perfect for connecting gestures → AI actions → device control

**Architecture:**

```
[Wristband] ─BLE─> [Phone/Hub] ─MCP─> [ChatGPT/Claude] ─API─> [Device APIs]
                         │
                         └─> Local device control
```

**Implementation Options:**

1. **Option A: Phone as MCP Bridge (Recommended for Hackathon)**
   ```
   Wristband (ESP32)
        ├─> Sends gesture JSON via BLE
        └─> Phone App receives JSON
              ├─> MCP Client connects to ChatGPT/Claude
              ├─> AI interprets context + gesture
              └─> Executes action:
                    ├─> Call Uber API
                    ├─> Control HomeKit/Google Home
                    ├─> Send HTTP requests
                    └─> Bluetooth commands to devices
   ```

2. **Option B: Direct Wi-Fi (If time permits)**
   - ESP32-S3 connects to Wi-Fi
   - Sends HTTPS requests directly to MCP server
   - More complex but fully standalone

**MCP Tool Definitions for Your Gestures:**

```json
{
  "tools": [
    {
      "name": "call_uber",
      "description": "Call an Uber to current location",
      "gesture_trigger": "fist + shake",
      "parameters": {
        "destination": "home|work|custom"
      }
    },
    {
      "name": "control_light",
      "description": "Toggle light in pointed direction",
      "gesture_trigger": "point + close_fist",
      "parameters": {
        "direction": "detected_from_imu",
        "action": "toggle|on|off"
      }
    },
    {
      "name": "unlock_phone",
      "description": "Unlock smartphone",
      "gesture_trigger": "double_tap_wrist",
      "parameters": {}
    },
    {
      "name": "open_door",
      "description": "Send unlock signal to smart lock",
      "gesture_trigger": "twist_wrist_cw",
      "parameters": {
        "door_id": "detected_from_proximity"
      }
    }
  ]
}
```

---

## 📡 Communication Protocols

### 1. Bluetooth LE (Primary Method)

**Wristband as BLE Peripheral:**

```c
// ESP32 BLE Service Structure
Service UUID: "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
├── Characteristic: Gesture Data
│   ├── UUID: "beb5483e-36e1-4688-b7f5-ea07361b26a8"
│   ├── Properties: NOTIFY
│   └── Format: JSON
│       {
│         "gesture": "point_up",
│         "confidence": 0.95,
│         "timestamp": 1728396000,
│         "imu": {
│           "pitch": 45,
│           "roll": 0,
│           "yaw": 180
│         },
│         "pressure": [array of 60 values]
│       }
│
└── Characteristic: Command Response
    ├── UUID: "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"
    ├── Properties: WRITE, INDICATE
    └── Format: JSON
        {
          "action": "light_toggle",
          "status": "success",
          "device_id": "living_room_light_1"
        }
```

**Receiving Device (Phone App):**
- React Native / Flutter app
- Scans for OpenMuscle device
- Maintains BLE connection
- Routes gestures to appropriate APIs

### 2. Audio Communication ("Gibberlink")

**Use Case:** Device-to-device communication without pairing

**Implementation:**
- **Frequency Shift Keying (FSK)** encoding
- Transmit JSON data as audio tones
- Similar to old dial-up modems
- Range: ~2-5 meters

**Library Recommendation:**
- **Chirp SDK** (commercial but has free tier)
- **Quiet.js** (open source, JavaScript)
- **Custom DTMF** (simple but limited)

```python
# Example: Audio JSON transmission
def encode_gesture_to_audio(gesture_json):
    # Convert JSON to binary
    data = json.dumps(gesture_json).encode()
    # Encode as audio tones (FSK)
    audio_signal = fsk_encode(data, sample_rate=44100)
    # Play through speaker
    play_audio(audio_signal)
```

### 3. Wi-Fi Direct (Alternative)

- ESP32-S3 can create its own Wi-Fi network
- Other devices connect directly
- Send JSON over HTTP/WebSocket
- Good for demos with laptops

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    OPENMUSCLE WRISTBAND                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ 60x Pressure │─>│   ESP32-S3   │<─│  IMU Sensor  │     │
│  │   Sensors    │  │              │  │ (ICM-42688)  │     │
│  └──────────────┘  │  - ML Model  │  └──────────────┘     │
│                    │  - BLE Stack │                        │
│  ┌──────────────┐  │  - Gesture   │  ┌──────────────┐     │
│  │  Microphone  │─>│    Classifier│<─│   Speaker    │     │
│  │  (optional)  │  │              │  │  (optional)  │     │
│  └──────────────┘  └──────┬───────┘  └──────────────┘     │
│                           │                                │
│  ┌──────────────┐         │          ┌──────────────┐     │
│  │    OLED      │<────────┘          │   Haptic     │     │
│  │   Display    │                    │    Motor     │     │
│  └──────────────┘                    └──────────────┘     │
└───────────────────────────┬─────────────────────────────────┘
                            │ BLE / Wi-Fi / Audio
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                  SMARTPHONE / HUB (Bridge)                  │
│  ┌────────────────────────────────────────────────────────┐ │
│  │          MCP CLIENT (ChatGPT/Claude/Local AI)          │ │
│  │  - Interprets gesture + context                        │ │
│  │  - Makes decisions about actions                       │ │
│  │  - Routes commands to appropriate APIs                 │ │
│  └────────────┬──────────────┬────────────┬───────────────┘ │
│               │              │            │                 │
└───────────────┼──────────────┼────────────┼─────────────────┘
                │              │            │
                ↓              ↓            ↓
    ┌──────────────┐  ┌────────────┐  ┌──────────┐
    │  Smart Home  │  │  Phone OS  │  │  Cloud   │
    │   Devices    │  │  Controls  │  │   APIs   │
    │ - Lights     │  │ - Unlock   │  │ - Uber   │
    │ - Locks      │  │ - Apps     │  │ - etc    │
    │ - Thermostat │  │            │  │          │
    └──────────────┘  └────────────┘  └──────────┘
```

---

## 🛠️ Development Roadmap (36 Hours)

### Pre-Hackathon (Now - Before Event)

**Hardware Preparation:**
- [ ] Verify FlexGrid is fully assembled and functional
- [ ] Test basic sensor readings on all 60 channels
- [ ] Confirm ESP32-S3 firmware flashing works
- [ ] (Optional) Add microphone/speaker if going audio route
- [ ] 3D print/design comfortable wristband enclosure
- [ ] Charge all batteries

**Software Setup:**
- [ ] Set up development environment:
  - Arduino IDE or PlatformIO
  - ESP32-S3 board definitions
  - Required libraries (BLE, I2C, TensorFlow Lite)
- [ ] Clone OpenMuscle firmware repository
- [ ] Test basic BLE communication
- [ ] Set up Python environment for ML training:
  ```bash
  pip install tensorflow numpy pandas scikit-learn matplotlib
  ```

**Research & Planning:**
- [ ] Choose final gesture set (8-12 gestures)
- [ ] Decide on demo devices/actions (3-5 impressive ones)
- [ ] Prepare datasets (if any pre-training possible)

---

### Hackathon Timeline: 36 Hours

#### **Hours 0-8: Data Collection & Initial Setup**

**Hour 0-2: Setup & Calibration**
- Connect wristband, verify all sensors working
- Write data collection firmware
- Set up data logging (SD card or serial → computer)

**Hour 2-8: Gesture Data Collection**
- Record training data for each gesture
- 50-100 samples per gesture
- Multiple people if possible
- Label and organize data
- Format: CSV with columns [timestamp, sensor1-60, imu_x, imu_y, imu_z, roll, pitch, yaw, label]

#### **Hours 8-16: ML Model Training**

**Hour 8-10: Data Preprocessing**
- Clean and normalize data
- Feature engineering (moving averages, derivatives)
- Train/test split

**Hour 10-14: Model Training**
- Start with Random Forest (faster, good baseline)
- Then try TensorFlow Lite CNN if time permits
- Cross-validation
- Achieve >85% accuracy goal

**Hour 14-16: Model Optimization & Deployment**
- Convert to TFLite format
- Quantize for ESP32
- Flash to wristband
- Test inference speed and accuracy

#### **Hours 16-24: Integration & Device Control**

**Hour 16-18: BLE Communication**
- Implement BLE server on wristband
- Create phone app or use BLE debugging app
- Verify gesture JSON transmission

**Hour 18-22: MCP/API Integration**
- Set up MCP client (Python/Node.js on laptop for demo)
- OR integrate with ChatGPT API directly
- Implement 3-5 device control demos:
  1. Smart light control (Philips Hue / HTTP API)
  2. Phone unlock simulation (Bluetooth trigger)
  3. HTTP request to external API (Uber, Spotify, etc.)
  4. Smart lock control (if hardware available)
  5. Computer control (keyboard/mouse commands)

**Hour 22-24: Error Handling & Reliability**
- Add gesture confirmation (haptic feedback)
- Implement timeout and retry logic
- Battery optimization

#### **Hours 24-32: Polish & Demo Preparation**

**Hour 24-28: UI/UX**
- OLED display shows:
  - Current gesture detected
  - Action being executed
  - Connection status
- Haptic feedback for confirmations
- Visual/audio feedback on success

**Hour 28-32: Demo Scenario Creation**
- Create compelling demo narrative:
  1. Walk into "smart room"
  2. Point at light → turns on (with direction from IMU)
  3. Gesture to unlock phone
  4. Swipe gesture to open door
  5. Fist gesture to call Uber
  6. Talk to wristband → AI responds via speaker
- Record backup video in case live demo fails
- Prepare slide deck explaining tech

#### **Hours 32-36: Testing & Buffer**

**Hour 32-34: Stress Testing**
- Test all gestures 20+ times each
- Test battery life (should last 2+ hours)
- Test range (BLE distance)
- Test with multiple team members

**Hour 34-36: Final Polish & Contingencies**
- Fix any last-minute bugs
- Prepare plan B for demo failures
- Rest and prepare presentation
- Charge everything

---

## 📊 Key Metrics for Success

### Technical Metrics:
- ✅ Gesture recognition accuracy: >85%
- ✅ Inference latency: <100ms
- ✅ BLE connection stability: >95%
- ✅ Battery life: >2 hours continuous use
- ✅ Successfully control 3+ different device types

### Demo Impact Metrics:
- ✅ "Wow factor" - audience reaction
- ✅ Reliability - works >90% of demo attempts
- ✅ Speed - faster than voice commands
- ✅ Novelty - clear differentiation from existing solutions

---

## 💡 Hackathon Presentation Strategy

### Story Arc:

1. **The Problem (30 sec)**
   - "Everyone talks about talking to AI"
   - "But sometimes silence is golden"
   - "What if you could control your world with simple gestures?"

2. **The Solution (1 min)**
   - "OpenMuscle wristband reads muscle activity"
   - "60 sensors detect finger and wrist movements"
   - "AI interprets gestures in real-time"
   - "MCP protocol connects to any service"

3. **Live Demo (3-4 min)**
   - Demo 3-5 impressive use cases
   - Show different gesture types
   - Emphasize speed and silence
   - Include one "AI reasoning" example (MCP + ChatGPT)

4. **Technical Deep Dive (1-2 min)**
   - Show architecture diagram
   - Mention ML accuracy stats
   - Highlight extensibility (any device, any gesture)

5. **Vision & Impact (30 sec)**
   - Accessibility (for people who can't/won't speak)
   - Privacy (no always-listening microphones)
   - Future: AR glasses + wristband = perfect combo
   - "The next interface is no interface"

### Demo Script Example:

```
[Walking on stage wearing wristband]

"Good morning. Watch this."

[Point at light] → Light turns on
[Point other direction] → Light turns off

"No voice. No app. Just gesture."

[Make calling gesture]

"I just called an Uber. It'll be here in 4 minutes."

[Face phone, make unlock gesture]

"My phone is now unlocked."

[Approach door, twist wrist]

[Door clicks open]

"This wristband reads the electrical activity in my forearm muscles.
60 sensors detect micro-movements before my fingers even move.
An AI model trained in 36 hours recognizes 12 different gestures.
Through MCP, it connects to ChatGPT, which interprets context 
and executes actions across any device or service.

The future of AI interaction isn't louder. It's silent."
```

---

## 🔗 Key Resources & Links

### OpenMuscle Resources:
- **Main Hub:** https://github.com/Open-Muscle/OpenMuscle-Hub
- **FlexGrid Hardware:** Your forked repo ✓
- **Firmware:** https://github.com/Open-Muscle/OpenMuscle-Firmware
- **MuscleSenseNet Model:** https://huggingface.co/openmuscle/muscle_sense_net
- **Community:** Discord @ https://discord.gg/WstCaqUG63
- **YouTube:** @openmuscle / @turfptax

### ML/AI Resources:
- **FS-HGR Paper (Few-Shot Learning):** https://arxiv.org/abs/2011.06104
- **ViT-HGR (Vision Transformer for EMG):** https://arxiv.org/abs/2201.10060
- **TensorFlow Lite for Microcontrollers:** https://www.tensorflow.org/lite/microcontrollers
- **Edge Impulse (Alternative ML platform):** https://edgeimpulse.com

### Communication & Integration:
- **ESP32 BLE Library:** Arduino BLE / ESP-IDF
- **MCP Documentation:** https://modelcontextprotocol.io
- **OpenAI API (for ChatGPT):** https://platform.openai.com
- **Home Assistant (Smart Home):** https://www.home-assistant.io
- **Uber API:** https://developer.uber.com

### Hardware Components (If Adding):
- **INMP441 I2S Microphone:** AliExpress / Amazon (~$3)
- **MAX98357A I2S Amplifier:** Adafruit #3006 (~$5)
- **Small 8Ω Speaker:** Any electronics store (~$2)

---

## 🎯 Winning Strategy

### What Makes This Project Stand Out:

1. **Novel Interaction Paradigm**
   - First to combine EMG + MCP + ChatGPT
   - Addresses real UX problem (voice fatigue)
   - Silent interaction is genuinely useful

2. **Technical Sophistication**
   - Custom hardware (60-sensor array)
   - On-device ML inference
   - Multiple communication protocols
   - Real integration with external APIs

3. **Practical Applications**
   - Accessibility (motor impairments, speech difficulties)
   - Privacy (no ambient listening)
   - Industrial (noisy environments)
   - Medical (sterile environments where touch is limited)

4. **Polish & Presentation**
   - Live demo that actually works
   - Clear narrative
   - Professional execution
   - Extensible architecture

### Potential Pitfalls to Avoid:

❌ **Over-complexity:** Don't try to support 50 gestures
✅ **Focus:** 8-12 gestures that work reliably

❌ **ML Perfectionism:** Spending 20 hours tuning for 2% accuracy gain
✅ **Pragmatism:** 85% accuracy + great demo > 95% accuracy + no integration

❌ **Live Demo Only:** What if WiFi fails?
✅ **Backup Plan:** Pre-recorded video + local demo without internet

❌ **Technical Jargon:** "Our CNN achieves 0.03 validation loss..."
✅ **Human Story:** "You can control your home with a thought"

---

## 🚀 Next Steps (Action Items)

### This Week:
1. Order any additional components (mic/speaker) - arrive before hackathon
2. Test FlexGrid sensor readings - verify all 60 channels work
3. Set up development environment on your laptop
4. Clone all necessary repos
5. Test basic BLE communication between ESP32 and phone
6. Decide on final 8-12 gesture set
7. Identify which devices you'll demo (borrow smart bulb, etc.)

### At Hackathon Start:
1. Immediate team roles assignment:
   - ML Engineer: Model training
   - Firmware Dev: ESP32 code
   - Integration Dev: Phone app + APIs
   - Demo Designer: UX + presentation
2. Set up shared GitHub repo for collaboration
3. Begin data collection within first 2 hours
4. Checkpoint meetings every 6 hours

### Emergency Contacts:
- OpenMuscle Discord community (real-time help)
- TensorFlow Lite documentation
- Stack Overflow tags: esp32, tensorflow-lite, ble

---

## 📝 Final Thoughts

This is a genuinely innovative concept that addresses a real gap in human-AI interaction. The technology exists, your hardware is ready, and 36 hours is enough time if you stay focused.

**Remember:**
- Demo > Perfect code
- Working prototype > Grand vision
- 3 reliable features > 10 buggy ones
- Story + emotion > Technical specs

**You've got this!** 🚀

The combination of OpenMuscle's sophisticated hardware, modern ML techniques, and MCP integration puts you in a unique position to create something truly memorable.

Good luck at Platanus! 🎉
