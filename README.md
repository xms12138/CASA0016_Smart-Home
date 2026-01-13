

# 🏡 HomeCare Assist  

**Smart-Home Prototype for Elderly Comfort & Safety**  

A lightweight Arduino-based system for indoor comfort monitoring and automated actuation.

---

## ✨ Overview  

**HomeCare Assist** is a room-level smart prototype designed to support **elderly individuals living alone**, helping them maintain a safer and more comfortable indoor environment—**without cameras, wearables, or user intervention**.

The system continuously monitors:

- Indoor **CO₂ / temperature / humidity**
- Indoor & outdoor **light levels**
- **Thermal-based occupancy** 
- Potential **fire hazards**

And automatically:

- Opens/closes curtains based on natural light
- Activates ventilation when air quality/temperature rises
- Triggers alarms when fire is detected
- Publishes live data via **MQTT** for remote monitoring & manual control

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/appearance.jpg" width="75%">
  <br>
  <strong>Figure 1.</strong> Physical Prototype Overview
</p>

---

## 🧩 Key Features  

- ✅ Privacy-friendly **thermal presence detection** (no camera)
- ✅ Real-time **multi-sensor monitoring**
- ✅ **Automatic ventilation & curtain control**
- ✅ **MQTT remote supervision + manual override**
- ✅ Local **TFT dashboard UI**
- ✅ Low-power design (suitable for standalone power bank)

------

## 🚀 Installation & Usage

### 1️⃣ Clone

```
git clone https://github.com/xms12138/CASA0016_Smart-Home.git
```

**The complete code of this system is located in the [Final](https://github.com/xms12138/CASA0016_Smart-Home/tree/main/Code/Final). If you just want to give it a try, all you need to do is run this code.**

**Individual debugging sketches for each component are available in the [Components_Test](https://github.com/xms12138/CASA0016_Smart-Home/tree/main/Code/Components_Test) folder and can be run independently if required.**

### 2️⃣ Arduino Libraries

Install these in Arduino IDE:

- Adafruit MLX90640
- Adafruit SCD30
- TFT_eSPI
- WiFiNINA
- PubSubClient
- Servo

### 3️⃣ Configure WiFi + MQTT

Edit in your Arduino sketch:

```
char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

const char* mqttServer = "Your_Server";
const int mqttPort = "Your_Port";
```

### 4️⃣ Upload & Run

- Wire all sensors following the schematic
- Upload sketch to **Arduino MKR WiFi 1010**
- Verify TFT screen updates
- Use an MQTT client (e.g. EasyMQTT) to publish fan commands

------

## 📂 Recommended Repository Structure

```
CASA0016_Smart-Home/
│
├── 3D-Printer/    – 3D model files for the device enclosure and printed parts
│
├── Code/          – Arduino firmware and source code for sensors, actuators and MQTT
│
├── Picture/       – Photos, wiring images, UI screenshots and documentation images
│
└── website/       – Web dashboard (HTML/CSS/JS) for visualizing device data

```

Each folder corresponds to a core part of the smart home prototype:

- **3D-Printer** → physical housing & printed components
- **Code** → main hardware logic running on the microcontroller
- **Picture** → project images and build documentation
- **website** → the browser-based dashboard for monitoring

---

## 🛠 Hardware Used  

| Component                       | Purpose                                 |
| ------------------------------- | --------------------------------------- |
| Arduino MKR WiFi 1010           | Main controller, WiFi connectivity      |
| MLX90640 (32×24 Thermal Camera) | Human presence detection                |
| SCD30                           | CO₂ + Temperature + Humidity monitoring |
| Light sensor (LDR)              | Ambient light measurement               |
| Flame sensor                    | Fire detection                          |
| MG90S Servo                     | Simulated curtain control               |
| 5V Fan                          | Auto / remote ventilation               |
| TFT_eSPI Display (480×320)      | On-device dashboard UI                  |

---

## 🧱 System Architecture  

The system runs on an Arduino MKR WiFi 1010 and integrates five key modules:

 MLX90640 for thermal presence detection, SCD30 for CO₂/temperature/humidity, an LDR for light sensing, a flame sensor for fire detection, and actuators including a PWM-controlled fan and a servo-driven curtain. All readings are displayed locally on a TFT screen and trigger automatic responses. The circuit diagram is as follows:

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/circuit_diagram.png" width="75%">
  <br>
  <strong>Figure 2.</strong> Circuit Diagram
</p>

The component diagram is as follows:

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/components.png" width="75%">
  <br>
  <strong>Figure 3.</strong> Components Diagram
</p>

------

## 🧪 Software Logic and Core Function

The system first determines occupancy using the thermal imager. If empty, all actuators stay idle. When occupied, rule-based thresholds control devices: CO₂ and temperature trigger the fan, light opens the curtains, and a flame event raises an alarm. This ensures predictable behaviour across all sensors. The workflow diagram is as follows:

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/system flowchart.png" width="75%">
  <br>
  <strong>Figure 4.</strong> System Flowchart
</p>

### 🔥 Thermal Presence Detection (MLX90640)

Presence is inferred using an adaptive, statistics-based method rather than fixed thresholds:

- Compute global mean **μ** and standard deviation **σ** per 32×24 frame
- Mark “hot” pixels by **z-score thresholding**
- Aggregate into **8×6 blocks** to suppress isolated noise
- Require global contrast check (**Tmax − μ**)
- Apply **temporal smoothing** to stabilize decisions

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/Use Python to render thermal imaging images/thermal images.png" width="75%">
  <br>
  <strong>Figure 5.</strong> Use Python to render thermal imaging images
</p>

------

### 🌬 Automatic & Remote Fan Control

Automatic mode runs only when presence is detected:

- Fan ON if **CO₂ > 5000 ppm** OR **Temperature > 29°C**
- MQTT remote control can force ON/OFF using:
  - Topic: `student/MUJI/HZH/command/fan`
  - Payload: `"on"` / `"off"`

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/Remote control and monitoring interface.png" width="75%">
  <br>
  <strong>Figure 6.</strong> Remote control and monitoring interface
</p>

Example JSON payload (remote monitoring):

```
{
  "temperature": 29.1,
  "humidity": 37.3,
  "presence": false,
  "fan_source": "auto",
  "fan": false,
  "fire": false,
  "curtain": "unchanged",
  "light": 332,
  "co2": 1021.8
}

```

------

### 🖥 Local TFT Dashboard + Website page

The local TFT UI shows:

- Presence status
- CO₂ ppm
- Temperature & Humidity
- Light level
- Status bar for **Fan / Fire / Sound / Curtain**

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/screen1.jpg" width="75%">
  <br>
  <strong>Figure 7.</strong> TFT Dashboard
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/xms12138/CASA0016_Smart-Home/main/Picture/website.png" width="75%">
  <br>
  <strong>Figure 8.</strong> Website Page
</p>

------

## 📊 Evaluation Summary

- Prototype achieved core objectives: sensing + control + MQTT monitoring
- Low-power test: **20,000 milliampere power bank powered system for > 1 week**
- All modules worked reliably in development environment

------

## 🔧 Limitations & Future Work

Limitations observed:

- Thermal detection can be affected by background heat sources and ambient temperature similarity
- Thresholds are coarse and not universal across rooms/seasons
- Too many modules reduce robustness (wiring looseness causes failures)

Future improvements:

- Simplify sensing setup (reduce redundant hardware)
- Replace fixed thresholds with adaptive logic
- Collect data for lightweight ML-based decision rules

------

## 📄 License  

This project is released under the MIT License.

------

## 📄References

Bouazizi, M., Ye, C. and Ohtsuki, T. (2022) ‘Low-resolution infrared array sensor for counting and localizing people indoors: When low end technology meets cutting edge deep learning techniques’, *Information*, 13(3), 132. doi:10.3390/info13030132.

 Du, B. et al. (2020) ‘Indoor CO₂ concentrations and cognitive function: A critical review’, *Indoor Air*, 30(6), pp. 1005–1016. Available at: [https://pubmed.ncbi.nlm.nih.gov/32557862/](https://pubmed.ncbi.nlm.nih.gov/32557862/?utm_source=chatgpt.com)

 Fan, Y. et al. (2023) ‘Short-term exposure to indoor carbon dioxide and human cognitive performance’, *Building and Environment*, 243, 110682. Available at: [https://www.sciencedirect.com/science/article/pii/S036013232300358X](https://www.sciencedirect.com/science/article/pii/S036013232300358X?utm_source=chatgpt.com)

 Lin, Y. and Zhao, Q. (2025) ‘Human occupancy monitoring and positioning with speed-responsive adaptive sliding window using an infrared thermal array sensor’, *Sensors*, 25(1), 129. doi:10.3390/s25010129.

 Jo, T.H., Ma, J.H. and Cha, S.H. (2021) ‘Elderly perception on the Internet of Things-based integrated smart-home system’, *Sensors*, 21(4), 1284. Available at: [https://www.mdpi.com/1424-8220/21/4/1284](https://www.mdpi.com/1424-8220/21/4/1284?utm_source=chatgpt.com)

 Wang, Y., Zhang, X., Zhang, Y. and Wang, K. (2021) ‘Multisensor fusion in smart buildings: A review’, *Automation in Construction*, 125, 103600. Available at: https://www.sciencedirect.com/science/article/pii/S0926580521001791

