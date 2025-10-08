# 🚀 OpenMuscle Gesture Control - Quick Start Guide

Get your gesture control system up and running in under 2 hours!

---

## ⚡ Pre-Hackathon Checklist

### Hardware
- [ ] OpenMuscle FlexGrid assembled and functional
- [ ] ESP32-S3 flashing verified
- [ ] LiPo battery charged
- [ ] USB-C cable for programming
- [ ] (Optional) Microphone and speaker modules
- [ ] (Optional) 3D-printed enclosure

### Software
- [ ] Arduino IDE or PlatformIO installed
- [ ] ESP32-S3 board support added
- [ ] Python 3.8+ installed
- [ ] Development laptop with Bluetooth

### Accounts & API Keys
- [ ] OpenAI API key (for MCP integration)
- [ ] Smart home device access (Hue, Home Assistant, etc.)
- [ ] GitHub account for collaboration

---

## 🛠️ Setup (Step by Step)

### Step 1: Clone Repository (5 minutes)

```bash
# Clone your forked FlexGrid repo (already done!)
cd /workspace

# Install Python dependencies
pip install -r requirements.txt

# Test Python environment
python -c "import numpy, tensorflow, bleak; print('✓ All imports successful')"
```

### Step 2: Flash Data Collection Firmware (15 minutes)

```bash
# Open Arduino IDE
# File → Open → firmware/gesture_data_collector.ino

# Configure board:
# Tools → Board → ESP32 Arduino → ESP32S3 Dev Module
# Tools → USB CDC On Boot → Enabled
# Tools → Upload Speed → 921600
# Tools → USB Mode → Hardware CDC and JTAG

# Connect wristband via USB-C
# Tools → Port → (select your COM/ttyUSB port)

# Upload firmware
# Click "Upload" button (or Ctrl+U)
```

**Verify it works:**
1. Open Serial Monitor (115200 baud)
2. Type `TEST` and press Enter
3. You should see sensor readings

### Step 3: Collect Training Data (2-3 hours)

```bash
# In Serial Monitor:
LABEL point_up
START
# Perform gesture 50 times
STOP

LABEL point_down
START
# Perform gesture 50 times
STOP

# Repeat for all gestures:
# - point_left
# - point_right
# - fist_close
# - fist_open
# - pinch
# - swipe_left
# - swipe_right
# - twist_cw
# - twist_ccw
# - idle (no gesture)
```

**Save data:**
1. Copy entire Serial Monitor output
2. Save as `collected_data.csv`
3. Move to `ml_training/data/` directory

### Step 4: Train ML Model (2-3 hours)

```bash
cd ml_training

# Option A: Quick Random Forest (recommended for hackathon)
python train_gesture_model.py \
    --data data/collected_data.csv \
    --model_type random_forest \
    --output_dir output/

# Option B: More accurate CNN (if time permits)
python train_gesture_model.py \
    --data data/collected_data.csv \
    --model_type cnn \
    --output_dir output/ \
    --tflite
```

**Expected output:**
```
Test Accuracy: 0.8942 (89.42%)
Model saved to: output/gesture_model_rf.pkl
```

### Step 5: Flash Gesture Recognition Firmware (15 minutes)

```bash
# Open firmware/gesture_recognition_ble.ino in Arduino IDE

# If using Random Forest:
# 1. Copy output/gesture_model_rf.pkl
# 2. Convert to C array (see conversion script)
# 3. Paste into firmware

# If using TFLite:
# 1. Copy output/gesture_model.tflite to Arduino sketch folder
# 2. It will be automatically included

# Upload firmware
```

### Step 6: Test BLE Communication (30 minutes)

```bash
# Terminal 1: Start BLE receiver
cd integration
python ble_receiver.py --device "OpenMuscle-FlexGrid"

# You should see:
# ✓ Found device: OpenMuscle-FlexGrid (XX:XX:XX:XX:XX:XX)
# ✓ Connected
# ✓ Subscribed to gesture notifications
# Waiting for gestures...

# Perform gestures on wristband
# Watch terminal for detected gestures
```

### Step 7: Configure Device Integration (1 hour)

```bash
# Create config file
cp integration/config.example.json integration/config.json

# Edit with your API keys:
nano integration/config.json
```

```json
{
  "openai_api_key": "sk-your-key-here",
  "philips_hue_bridge": "192.168.1.100",
  "philips_hue_token": "your-hue-token",
  "home_assistant_url": "http://homeassistant.local:8123",
  "home_assistant_token": "your-ha-token"
}
```

**Test device control:**

```bash
# Test Philips Hue
curl -X PUT http://192.168.1.100/api/YOUR-TOKEN/lights/1/state \
  -d '{"on":true}'

# Test Home Assistant
curl -X POST http://homeassistant.local:8123/api/services/light/turn_on \
  -H "Authorization: Bearer YOUR-TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"entity_id": "light.living_room"}'
```

---

## 🎯 Hackathon Workflow (36 Hours)

### Phase 1: Foundation (Hours 0-8)

**Hour 0-2: Setup & Verification**
- [ ] Hardware check
- [ ] Software environment ready
- [ ] Team roles assigned

**Hour 2-6: Data Collection**
- [ ] Collect 50+ samples per gesture
- [ ] 3 team members = 150 samples per gesture
- [ ] Save CSV file

**Hour 6-8: Initial Training**
- [ ] Train Random Forest model
- [ ] Achieve >80% accuracy
- [ ] If accuracy low, collect more data

### Phase 2: Integration (Hours 8-16)

**Hour 8-10: Firmware Development**
- [ ] Flash gesture recognition firmware
- [ ] Test BLE communication
- [ ] Verify gesture detection

**Hour 10-14: Device Integration**
- [ ] Set up smart home APIs
- [ ] Test each device type
- [ ] Implement 3-5 actions

**Hour 14-16: MCP Integration**
- [ ] Add OpenAI function calling
- [ ] Test AI decision making
- [ ] Refine prompts

### Phase 3: Polish (Hours 16-24)

**Hour 16-20: User Experience**
- [ ] OLED display messages
- [ ] Haptic feedback tuning
- [ ] Error handling

**Hour 20-22: Demo Preparation**
- [ ] Create demo script
- [ ] Test demo flow 10+ times
- [ ] Record backup video

**Hour 22-24: Contingency**
- [ ] Fix any critical bugs
- [ ] Prepare Plan B demos
- [ ] Charge all devices

### Phase 4: Presentation (Hours 24-36)

**Hour 24-30: Slide Deck**
- [ ] Problem statement
- [ ] Solution overview
- [ ] Technical architecture
- [ ] Live demo
- [ ] Future vision

**Hour 30-34: Practice**
- [ ] Rehearse presentation 3+ times
- [ ] Time each section
- [ ] Prepare for Q&A

**Hour 34-36: Final Prep**
- [ ] Rest!
- [ ] Check all equipment
- [ ] Arrive early at venue

---

## 🎬 Demo Day Checklist

### Equipment
- [ ] Charged wristband
- [ ] Charged laptop
- [ ] Phone/tablet for demo
- [ ] HDMI cable for projector
- [ ] Backup battery pack
- [ ] Smart bulb or demo devices

### Software
- [ ] All services running
- [ ] Backup video ready
- [ ] Presentation slides loaded

### Team
- [ ] Presenter prepared
- [ ] Technical support ready
- [ ] Demo operator assigned

---

## 🐛 Common Issues & Solutions

### Issue: Wristband not connecting via BLE

**Solutions:**
1. Check Bluetooth is enabled on laptop
2. Restart wristband (press reset button)
3. Clear Bluetooth cache: `sudo systemctl restart bluetooth`
4. Check device name matches: "OpenMuscle-FlexGrid"
5. Try closer proximity (<2 meters)

### Issue: Low gesture recognition accuracy (<80%)

**Solutions:**
1. Collect more training data (100+ samples per gesture)
2. Check sensor connections (all 60 channels working?)
3. Add more diverse users to training set
4. Increase confidence threshold in firmware
5. Simplify gesture set (remove similar gestures)

### Issue: Sensor readings all zero

**Solutions:**
1. Check multiplexer wiring
2. Verify ADC pin configuration
3. Test with `TEST` command in data collector
4. Check solder joints on flex PCB

### Issue: IMU not responding

**Solutions:**
1. Check I2C connections (SDA, SCL)
2. Scan I2C bus: `i2cdetect -y 1`
3. Verify IMU address (0x68 or 0x69)
4. Check IMU library version

### Issue: Smart devices not responding

**Solutions:**
1. Verify API keys are correct
2. Check network connectivity
3. Test API calls with curl
4. Add verbose logging
5. Check firewall rules

### Issue: OpenAI API rate limit

**Solutions:**
1. Implement local caching
2. Add simple rule-based fallback
3. Upgrade API tier
4. Batch multiple gestures

---

## 📊 Testing Protocol

### Unit Tests

```bash
# Test sensor reading
python -m pytest tests/test_sensors.py

# Test ML model
python -m pytest tests/test_model.py

# Test BLE communication
python -m pytest tests/test_ble.py
```

### Integration Tests

```bash
# End-to-end gesture test
python tests/test_e2e_gesture.py

# Expected output:
# ✓ Gesture detected: point_up
# ✓ Action executed: light_on
# ✓ Device responded: success
```

### Load Testing

```bash
# Test rapid gesture detection
python tests/stress_test.py --gestures 100 --rate 2

# Verify:
# - No dropped gestures
# - Latency < 200ms
# - No memory leaks
```

---

## 💡 Pro Tips

1. **Start Simple**: Get 3 gestures working perfectly before adding more
2. **Test Early**: Don't wait until hour 30 to test live demo
3. **Plan B**: Have fallback for every demo component
4. **Battery**: Charge everything overnight before demo day
5. **Team Communication**: Use Discord/Slack for real-time coordination
6. **Documentation**: Take photos/videos throughout development
7. **Git Commits**: Commit working states frequently
8. **Sleep**: 4 hours sleep on night 1 = better performance day 2

---

## 📞 Help & Resources

### Getting Stuck?
- **OpenMuscle Discord**: https://discord.gg/WstCaqUG63
- **Stack Overflow**: Tag `esp32`, `tensorflow-lite`, `ble`
- **GitHub Issues**: Post in your forked repo

### Learning Resources
- ESP32 BLE: https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
- TensorFlow Lite: https://www.tensorflow.org/lite/microcontrollers
- Home Assistant API: https://developers.home-assistant.io/docs/api/rest/

---

## 🏆 Success Criteria

By end of hackathon, you should have:

✅ **Working Prototype**
- Detects 8+ gestures with >85% accuracy
- BLE communication stable
- Controls 3+ different device types

✅ **Impressive Demo**
- 3-5 minute presentation
- Live demonstrations work
- Clear value proposition

✅ **Technical Depth**
- ML model trained and optimized
- MCP/AI integration functional
- Clean architecture

✅ **Innovation**
- Novel interaction paradigm
- Practical use cases
- Extensible design

---

## 🚀 You're Ready!

This is an ambitious project, but absolutely achievable in 36 hours with proper planning. Focus on getting the core loop working (gesture → detection → action) in the first 12 hours, then iterate on accuracy and features.

**Remember:** A working demo with 5 gestures beats a perfect plan with 20 gestures!

Good luck! 🎉

---

**Questions during hackathon?**
Check the Discord or create GitHub issues in your forked repo.

**After hackathon?**
Share your results! The OpenMuscle community would love to see what you built.
