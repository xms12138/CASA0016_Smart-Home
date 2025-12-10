# CASA0016_Smart-Home
# Smart Room Dashboard

**An IoT-based intelligent room monitoring and automation system powered by Arduino MKR WiFi 1010, thermal imaging, environmental sensors, local visualization, and MQTT integration.**

------

## 📌 Overview

**Smart Room Dashboard** is a fully integrated smart-environment system designed to detect human presence, evaluate indoor air quality, monitor environmental risks, and automatically control room actuators such as a fan, curtain servo, and buzzer alarm.

The system runs on an **Arduino MKR WiFi 1010**, combines multiple sensors including **thermal imaging (MLX90640)** and **CO₂/temperature/humidity (SCD30)**, and presents real-time room status on a **TFT display** while sending structured JSON data to an MQTT server. It also accepts remote fan-control commands from mobile or cloud applications.

This project demonstrates **real-time sensing, embedded automation, human detection, local UI design, and IoT communication** in a single device.

------

## ✨ Features

### 🔥 Human Presence Detection (MLX90640 Thermal Camera)

- Reads a 32×24 thermal frame.
- Computes frame average / max temperature dynamically.
- Detects humans using:
  - Hot-pixel count
  - Temperature delta threshold
- Robust to noise and background variations.

### 🌬 Automatic + Remote Fan Control

- Auto mode activates fan when:
  - CO₂ > **5000 ppm**, or
  - Temperature > **29°C**.
- Remote mode overrides auto (via MQTT command).
- Fan state and control source displayed clearly on the screen.

### 🌡 Environmental Monitoring (SCD30)

- Measures **CO₂**, **temperature**, and **humidity**.
- Used for decision-making and screen display.
- Included in MQTT JSON payloads.

### ☀️ Light-Based Curtain Automation

- Reads ambient light from an analog sensor.
- Human present:
  - Bright → open curtain (servo → 180°)
  - Dim → close curtain (servo → 0°)
- Human absent:
  - Curtain remains unchanged.

### 🔥 Fire Detection & Alarm

- Fire sensor triggers alarm when HIGH.
- Activates buzzer for **5 seconds**.
- Fire status reflected on TFT display.

### 🎧 Sound-Level Monitoring (Adaptive Baseline)

- Smooth, noise-resistant measurement using:
  - Fast averaging
  - Adaptive baseline
  - Exponential smoothing
- Useful for noise alerting or ambient analysis.

### 📺 High-Quality Local UI (TFT_eSPI)

- Live dashboard with:
  - Human presence
  - CO₂ / temperature / humidity
  - Light level
  - Sound level
- Bottom status bar with visual blocks:
  - FAN / FIRE / SOUND / CURTAIN

### 📡 MQTT JSON Telemetry + Remote Control

- Periodic JSON payload includes all sensor & actuator states.

- Publishes to:

  ```
  student/MUJI/HZH
  ```

- Remote fan command topic:

  ```
  student/MUJI/HZH/command/fan
  ```

- Accepts `"on"` / `"off"` commands from mobile apps, dashboards, or cloud services.

------

## 🛠 Hardware Components

| Component                              | Purpose                            |
| -------------------------------------- | ---------------------------------- |
| **Arduino MKR WiFi 1010**              | Main controller, WiFi connectivity |
| **MLX90640 32×24 Thermal Camera**      | Human presence detection           |
| **SCD30 CO₂ + Temp + Humidity sensor** | Indoor air quality monitoring      |
| **Light sensor (LDR)**                 | Ambient light measurement          |
| **Sound sensor**                       | Adaptive sound-level monitoring    |
| **Flame sensor**                       | Fire detection                     |
| **MG90S Servo**                        | Simulated curtain control          |
| **5V Fan**                             | Auto / remote ventilation          |
| **Passive buzzer**                     | 5-second fire alarm                |
| **TFT_eSPI Display (480×320)**         | On-device dashboard UI             |

------

## 📑 Firmware Summary

The firmware implements:

### 1. Sensor Reading

- MLX90640 thermal frame extraction.
- SCD30 environmental data polling.
- Light / sound analog readings.
- Fire digital input.

### 2. Human Detection Algorithm

- Dynamic temperature thresholding.
- Hot-pixel count.
- Max/average difference validation.

### 3. Environment-Based Automation

- Fan auto-mode based on CO₂ + temperature.
- Curtain logic based on light + presence.
- Fire alarm handling with 5-second buzzer window.

### 4. User Interface Rendering

- Full-screen layout with:
  - Main data panel
  - Four status blocks

### 5. MQTT Communication

- Publishes telemetry JSON every loop.
- Handles remote fan commands via callback.

------

## 📤 Example MQTT JSON Output

```
{
  "presence": true,
  "co2": 5420.32,
  "temperature": 29.47,
  "humidity": 45.10,
  "fan": true,
  "fan_source": "auto",
  "light": 712,
  "curtain": "open",
  "fire": false,
  "buzzer": false,
  "sound_level": 37
}
```

------

## ▶️ How to Run the Project

### 1. Install Required Arduino Libraries

- **Adafruit MLX90640**
- **Adafruit SCD30**
- **WiFiNINA**
- **PubSubClient**
- **TFT_eSPI**
- **Servo**

### 2. Configure TFT_eSPI

Ensure `User_Setup_Select.h` selects the correct driver for your TFT display.

### 3. Flash the Provided Firmware

Upload the full `.ino` file (the one you provided).

### 4. Connect to WiFi & MQTT

The board will automatically:

- Join your WiFi
- Connect to `mqtt.cetools.org`
- Subscribe to the fan command topic
- Begin publishing telemetry

### 5. Optional: Send Remote Commands

Publish the following to control the fan:

| Topic                          | Payload          |
| ------------------------------ | ---------------- |
| `student/MUJI/HZH/command/fan` | `"on"` / `"off"` |

------

## 📦 Folder Structure (recommended)

```
SmartRoomDashboard/
│
├── firmware/
│   └── SmartRoomDashboard.ino
│
├── docs/
│   ├── system-architecture.png
│   ├── wiring-diagram.png
│   └── screenshots/
│
└── README.md
```

------

## 🚀 Future Improvements

- Add anomaly detection for thermal frames.
- Integrate WebSocket real-time dashboard.
- Add sleep/work/user profiles.
- LTE/WiFi dual-network fallback.
- Edge AI classification for improved presence detection.

------

## 📝 License

This project is released under the MIT License.
 You may use, modify, and distribute it freely.

------

## 🙌 Acknowledgments

Thanks to the open-source community and Adafruit libraries that made hardware integration possible.
