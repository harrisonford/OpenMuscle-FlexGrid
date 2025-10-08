# 🛒 Hardware Shopping List

## What You Already Have ✓

From your forked OpenMuscle FlexGrid repository, you have:
- ✓ FlexGrid 60-sensor pressure array (flex PCB)
- ✓ ESP32-S3-WROOM-1-N16R8 module
- ✓ CD74HC4067 multiplexer (16-channel)
- ✓ ICM-42688-P IMU (6-DOF)
- ✓ SSD1306 OLED display
- ✓ LiPo battery charging circuit
- ✓ Haptic feedback motor
- ✓ Buttons (Menu, Select, Boot, Reset)
- ✓ USB-C interface

**You're 90% ready!** The core hardware is complete.

---

## Optional Additions (For Enhanced Features)

### 1. Audio Components (For Voice & "Gibberlink")

#### Option A: Digital Microphone (Recommended)
| Component | Specs | Source | Price |
|-----------|-------|--------|-------|
| **INMP441 I2S Microphone** | - Digital MEMS mic<br>- I2S interface<br>- Low noise<br>- 24-bit | [AliExpress](https://www.aliexpress.com/item/32961861654.html)<br>[Amazon](https://www.amazon.com/INMP441-Omnidirectional-Microphone-Interface-Precision/dp/B08K8RZ6YC) | $3-5 |

**Why:** Clean digital signal, native ESP32 support, low power

#### Option B: I2S Audio Amplifier + Speaker
| Component | Specs | Source | Price |
|-----------|-------|--------|-------|
| **MAX98357A I2S Amp** | - Class D amplifier<br>- 3W output<br>- I2S interface<br>- No volume control needed | [Adafruit #3006](https://www.adafruit.com/product/3006)<br>[Amazon](https://www.amazon.com/Adafruit-MAX98357-Amplifier-Breakout/dp/B07TGCM7SM) | $5-7 |
| **Small 8Ω Speaker** | - 3W power<br>- 8Ω impedance<br>- 30-40mm diameter | Any electronics store | $2-5 |

**Total for audio: $10-17**

**Alternative - Use Existing Haptic Motor:**
- Generate tones using PWM on haptic motor
- Free! (already have it)
- Limited frequency range but works for simple audio signals

---

### 2. Enhanced Power Options

| Component | Specs | Source | Price |
|-----------|-------|--------|-------|
| **Larger LiPo Battery** | - 1000-2000 mAh<br>- 3.7V single cell<br>- JST connector | [Adafruit](https://www.adafruit.com/category/574)<br>[Amazon](https://www.amazon.com/s?k=lipo+battery+1000mah) | $8-15 |
| **Backup Battery Pack** | - For demo day<br>- USB-C output<br>- 10000 mAh | Any store | $15-30 |

**Note:** Check current battery capacity. If >500mAh, you're good for hackathon.

---

### 3. Enclosure & Comfort

| Component | Purpose | Source | Price |
|-----------|---------|--------|-------|
| **Velcro Straps** | Secure wristband to forearm | Any fabric/craft store | $3-5 |
| **Foam Padding** | Comfort + sensor contact | Craft store | $2-3 |
| **Elastic Band** | Alternative strap | Fabric store | $2-4 |
| **3D Printed Case** | Professional look (optional) | 3D print service / own printer | $5-20 |

**Total for comfort: $10-30**

---

### 4. Demo Equipment

| Item | Purpose | Where to Get | Price |
|------|---------|--------------|-------|
| **Smart Light Bulb** | Demo device control | - Philips Hue ($15-25)<br>- Wyze Bulb ($10)<br>- Any WiFi bulb | $10-25 |
| **Smart Plug** | Control any device | - TP-Link Kasa ($10)<br>- Wyze Plug ($6) | $6-15 |
| **HDMI Cable** | Connect to projector | Any electronics store | $5-10 |
| **Extension Cord** | Demo setup | Any store | $5-10 |

**Total for demo: $25-60**

**Alternative - Free Demo Options:**
- Control laptop/computer (keyboard shortcuts via USB)
- Phone app controls (free apps)
- Simulated device control (visual on screen)

---

### 5. Development Tools

| Tool | Purpose | Source | Price |
|------|---------|--------|-------|
| **USB-C Cable** | Programming + charging | Any electronics store | $5-10 |
| **Breadboard Jumper Wires** | Testing connections | Amazon/AliExpress | $3-5 |
| **Multimeter** | Debugging hardware | Amazon | $10-20 |
| **Soldering Iron** | Repairs/mods (if needed) | Amazon | $20-40 |

**Already have these? Skip!**

---

## Recommended Purchase Priorities

### Tier 1 - Essential for Hackathon Success
1. **Nothing!** Your hardware is complete ✓
2. Maybe: Velcro straps for comfort ($3-5)
3. Maybe: Smart bulb for impressive demo ($10-25)

### Tier 2 - Nice to Have
1. Audio components (mic + speaker) - $10-17
2. Larger battery for all-day demos - $8-15
3. Demo equipment - $25-60

### Tier 3 - Can Skip
1. 3D printed enclosure (use cardboard/tape for hackathon)
2. Extra development tools (probably have these)

---

## Budget Scenarios

### **Minimum Budget: $0-10**
- Use existing FlexGrid hardware ✓
- Tape/velcro for strapping
- Demo on computer (free)
- **Total: You're ready to go!**

### **Recommended Budget: $30-50**
- Velcro straps: $5
- Smart bulb demo: $15
- Microphone module: $5
- Speaker + amp: $10
- Extension cord/cables: $5
- **Total: $40** - Professional demo setup

### **Premium Budget: $80-120**
- All above: $40
- Larger battery: $15
- 3D printed case: $20
- Multiple demo devices: $40
- Backup equipment: $20
- **Total: $135** - Full production-ready setup

---

## Where to Buy (By Speed)

### Same Day (Local Stores)
- **Velcro, straps, foam:** Craft stores (Michael's, Joann, etc.)
- **Smart bulbs:** Best Buy, Target, Walmart
- **Cables, batteries:** Best Buy, MicroCenter
- **Extension cords:** Home Depot, Lowe's, hardware stores

### 2-Day Shipping (Amazon Prime)
- All electronic components
- Smart home devices
- Development tools

### 1-2 Week Shipping (Cheaper)
- **AliExpress:** Best prices on microphones, components
- **Adafruit:** Quality electronics (1 week in US)
- **SparkFun:** Similar to Adafruit

---

## Alternative: Borrow/Use What You Have

### From Your Lab/Hackerspace
- [ ] Soldering equipment
- [ ] Multimeter
- [ ] Power supplies
- [ ] Spare cables

### From Home
- [ ] Phone charger (USB-C)
- [ ] Extension cord
- [ ] Duct tape/velcro
- [ ] Cardboard (enclosure)

### From Friends/Teammates
- [ ] Smart home devices to demo
- [ ] Laptop with Bluetooth
- [ ] Extra batteries

---

## Component Deep Dives

### Why INMP441 Microphone?

**Pros:**
- ✓ Digital output (no ADC noise)
- ✓ Native ESP32 I2S support
- ✓ Low power consumption
- ✓ Small size
- ✓ Good sensitivity

**Cons:**
- ✗ Requires I2S configuration
- ✗ More complex than analog mic

**Alternatives:**
- MAX9814 (analog, easier but noisier)
- MAX4466 (analog, adjustable gain)
- SPH0645 (similar to INMP441)

### Why MAX98357A Amplifier?

**Pros:**
- ✓ I2S interface (pairs with ESP32)
- ✓ No external components needed
- ✓ 3W power (loud enough)
- ✓ Efficient Class D

**Cons:**
- ✗ No volume control (fixed gain)
- ✗ Requires speaker

**Alternatives:**
- PAM8403 (analog input, cheaper)
- LM386 (classic, more components)
- Use Bluetooth speaker (wireless)

---

## Wiring Diagrams

### Adding INMP441 Microphone

```
ESP32-S3          INMP441
--------          -------
GPIO 14   <--->   SD (Data)
GPIO 15   <--->   WS (Word Select)
GPIO 16   <--->   SCK (Clock)
3.3V      <--->   VDD
GND       <--->   GND
```

### Adding MAX98357A + Speaker

```
ESP32-S3          MAX98357A          Speaker
--------          ---------          -------
GPIO 17   <--->   DIN (Data)
GPIO 18   <--->   BCLK (Bit Clock)
GPIO 19   <--->   LRC (Left/Right Clock)
5V        <--->   VIN
GND       <--->   GND
                  OUT+       <--->   Speaker +
                  OUT-       <--->   Speaker -
```

**Note:** These GPIO pins are suggestions. Check your FlexGrid schematic for available pins.

---

## Testing Your Additions

### Test Microphone
```cpp
// Arduino sketch to test INMP441
#include <driver/i2s.h>

void setup() {
  Serial.begin(115200);
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = 16,
    .ws_io_num = 15,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = 14
  };
  
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  
  Serial.println("Microphone initialized");
}

void loop() {
  int32_t sample = 0;
  size_t bytes_read = 0;
  
  i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
  
  // Print audio level
  Serial.println(abs(sample / 1000));
  delay(10);
}
```

**Expected:** Values change when you speak into mic

### Test Speaker
```cpp
// Play a test tone
#include <driver/i2s.h>

void playTone(int frequency, int duration) {
  // Generate sine wave samples
  int samples = 1000;
  int16_t wave[samples];
  
  for (int i = 0; i < samples; i++) {
    wave[i] = (int16_t)(32767 * sin(2 * PI * i / samples));
  }
  
  // Write to I2S
  size_t bytes_written;
  i2s_write(I2S_NUM_0, wave, sizeof(wave), &bytes_written, portMAX_DELAY);
}

void setup() {
  // I2S speaker initialization (similar to mic)
  playTone(440, 500);  // Play 440Hz (A note) for 500ms
}
```

**Expected:** Hear a tone from speaker

---

## Pre-Hackathon Preparation

### 1 Week Before
- [ ] Order any components you need
- [ ] Test all existing hardware
- [ ] Verify sensor readings

### 3 Days Before
- [ ] Receive and test new components
- [ ] Solder/assemble audio modules
- [ ] Test wristband fit and comfort

### 1 Day Before
- [ ] Full hardware check
- [ ] Charge all batteries
- [ ] Pack backup components
- [ ] Print wiring diagrams

---

## Component Storage & Transport

### Hackathon Kit Checklist
```
[ ] Wristband (in protective case)
[ ] USB-C cable
[ ] LiPo battery (charged)
[ ] Spare battery
[ ] Smart bulb for demo
[ ] Extension cord
[ ] Velcro straps
[ ] Duct tape (fixes everything!)
[ ] Multimeter (debugging)
[ ] Spare jumper wires
[ ] Phone/tablet (for control)
[ ] Laptop (development)
[ ] This documentation (printed!)
```

---

## Safety Notes

⚠️ **LiPo Battery Safety:**
- Never leave charging unattended
- Don't over-discharge (<3.0V)
- Store in LiPo safe bag
- Check for swelling/damage

⚠️ **Soldering Safety:**
- Use in ventilated area
- Don't touch hot iron
- Use helping hands tool

⚠️ **Electrical Safety:**
- Check polarity before connecting
- Use correct voltage (3.3V logic)
- Add inline fuses for power

---

## Summary: What to Buy NOW

**For Minimal Working Demo:**
- **Nothing!** You have everything ✓

**For Professional Demo:**
1. Velcro straps - $5 (comfort)
2. Philips Hue bulb - $15 (impressive demo)
3. USB-C cable - $5 (if you don't have one)

**Total: $25 - Ready for hackathon! 🚀**

Everything else is optional and can be added later for production.

---

Need help choosing? Feel free to ask in the OpenMuscle Discord!
