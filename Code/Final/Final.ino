#include <Wire.h>
#include <Adafruit_MLX90640.h>
#include <Adafruit_SCD30.h>
#include <Servo.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>

char ssid[] = "5339 Hyperoptic Fibre Broadband";
char pass[] = "3F38ikCLBkgT";

const char* mqttServer = "mqtt.cetools.org";
const int mqttPort = 1884;
const char* mqttUser = "student";
const char* mqttPass = "ce2021-mqtt-forget-whale";
const char* mqttTopic = "student/MUJI/HZH";
const char* mqttCmdFanTopic = "student/MUJI/HZH/command/fan";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

TFT_eSPI tft = TFT_eSPI();

#define COLOR_TITLE TFT_CYAN
#define COLOR_VALUE TFT_WHITE
#define COLOR_SECTION TFT_YELLOW
#define COLOR_LINE TFT_DARKGREY

Adafruit_MLX90640 mlx;
Adafruit_SCD30 scd30;
Servo curtainServo;

float frame[32 * 24];

const float TEMP_DELTA_THRESHOLD = 3.5;
const int MIN_HOT_PIXELS = 8;

const int FAN_PIN = 7;

const int LIGHT_PIN = A1;
const int LIGHT_THRESHOLD = 600;
const int SERVO_PIN = 6;

const int FLAME_PIN = 2;

const int BUZZER_PIN = 5;

float latestCO2 = NAN;
bool hasCO2 = false;

int currentServoAngle = -1;

bool buzzerOn = false;
unsigned long buzzerStartMillis = 0;
bool lastFireDetected = false;

int remoteFanMode = -1;

const int SOUND_PIN = A2;
float baselineAlpha = 0.98;
float soundAlpha = 0.85;
float baseline = 0;
int soundLevel = 0;

unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL_MS = 100;

int readFastAverage(int samples = 20) {
  long s = 0;
  for (int i = 0; i < samples; i++) {
    s += analogRead(SOUND_PIN);
    delay(2);
  }
  return (int)(s / samples);
}

bool detectHuman(const float *f) {
  float sum = 0.0;
  float minT = 0x7FFFFFFF;
  float maxT = -0x7FFFFFFF;

  for (int i = 0; i < 32 * 24; i++) {
    float t = f[i];
    sum += t;
    if (t < minT) minT = t;
    if (t > maxT) maxT = t;
  }

  float avg = sum / (32.0 * 24.0);
  float threshold = avg + TEMP_DELTA_THRESHOLD;

  int hotPixels = 0;
  for (int i = 0; i < 768; i++) {
    if (f[i] > threshold) hotPixels++;
  }

  bool presence = (hotPixels >= MIN_HOT_PIXELS) && ((maxT - avg) > TEMP_DELTA_THRESHOLD);

  Serial.print("IR avg="); Serial.print(avg, 2);
  Serial.print("  max="); Serial.print(maxT, 2);
  Serial.print("  hotPixels="); Serial.print(hotPixels);
  Serial.print("  presence=");
  Serial.println(presence ? "YES" : "NO");

  return presence;
}

void drawBaseLayout() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 480, 30, TFT_DARKGREY);
  tft.setTextColor(COLOR_TITLE, TFT_DARKGREY);
  tft.drawCentreString("Smart Room Dashboard", 240, 5, 2);

  tft.drawRoundRect(5, 35, 470, 190, 8, COLOR_LINE);
  tft.setTextColor(COLOR_SECTION, TFT_BLACK);
  tft.drawString("MAIN DATA", 10, 40, 2);

  int cellW = 480 / 4;
  tft.drawRect(0, 230, cellW, 90, COLOR_LINE);
  tft.drawRect(cellW, 230, cellW, 90, COLOR_LINE);
  tft.drawRect(cellW * 2, 230, cellW, 90, COLOR_LINE);
  tft.drawRect(cellW * 3, 230, cellW, 90, COLOR_LINE);

  tft.setTextColor(COLOR_SECTION, TFT_BLACK);
  tft.drawCentreString("FAN", cellW / 2, 235, 2);
  tft.drawCentreString("FIRE", cellW + cellW / 2, 235, 2);
  tft.drawCentreString("SOUND", cellW * 2 + cellW / 2, 235, 2);
  tft.drawCentreString("CURTAIN", cellW * 3 + cellW / 2, 235, 2);
}

void updateMainDataPanel(bool human, bool hasCO2, float co2, float temp, float humidity, int lightRaw, int soundLevel) {
  int xLabel = 15;
  int xValue = 170;
  int yStart = 65;
  int lineH = 25;

  tft.fillRect(10, 60, 460, 165, TFT_BLACK);
  tft.setTextColor(COLOR_VALUE, TFT_BLACK);

  tft.drawString("Human presence:", xLabel, yStart, 2);
  tft.drawString(human ? "YES" : "NO", xValue, yStart, 2);

  if (hasCO2) {
    tft.drawString("CO2:", xLabel, yStart + lineH, 2);
    tft.drawString(String(co2, 1) + " ppm", xValue, yStart + lineH, 2);

    tft.drawString("Temperature:", xLabel, yStart + lineH * 2, 2);
    tft.drawString(String(temp, 1) + " C", xValue, yStart + lineH * 2, 2);

    tft.drawString("Humidity:", xLabel, yStart + lineH * 3, 2);
    tft.drawString(String(humidity, 1) + " %", xValue, yStart + lineH * 3, 2);
  } else {
    tft.drawString("CO2 / Temp / Hum:", xLabel, yStart + lineH, 2);
    tft.drawString("Waiting SCD30 data...", xValue, yStart + lineH, 2);
  }

  tft.drawString("Light raw:", xLabel, yStart + lineH * 4, 2);
  tft.drawString(String(lightRaw), xValue, yStart + lineH * 4, 2);

  tft.drawString("Sound level:", xLabel, yStart + lineH * 5, 2);
  tft.drawString(String(soundLevel), xValue, yStart + lineH * 5, 2);
}

void drawFanStatus(bool fanOn, const String &fanSource) {
  int cellW = 480 / 4;
  int x = 0;
  int y = 255;
  int w = cellW;
  int h = 60;

  tft.fillRect(x + 2, y, w - 4, h, TFT_BLACK);

  uint16_t color = fanOn ? TFT_GREEN : TFT_DARKGREY;
  tft.fillRoundRect(x + 10, y + 5, w - 20, 20, 4, color);

  tft.setTextColor(COLOR_VALUE, TFT_BLACK);
  tft.drawCentreString(fanOn ? "ON" : "OFF", x + w / 2, y + 8, 2);
  tft.drawCentreString("(" + fanSource + ")", x + w / 2, y + 32, 2);
}

void drawFireStatus(bool fireDetected) {
  int cellW = 480 / 4;
  int x = cellW;
  int y = 255;
  int w = cellW;
  int h = 60;

  tft.fillRect(x + 2, y, w - 4, h, TFT_BLACK);

  if (fireDetected) {
    tft.fillRoundRect(x + 10, y + 5, w - 20, 40, 6, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawCentreString("FIRE!", x + w / 2, y + 16, 4);
  } else {
    tft.fillRoundRect(x + 10, y + 5, w - 20, 40, 6, TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawCentreString("SAFE", x + w / 2, y + 16, 4);
  }
}

void drawSoundStatus(int soundLevel) {
  int cellW = 480 / 4;
  int x = cellW * 2;
  int y = 255;
  int w = cellW;
  int h = 60;

  tft.fillRect(x + 2, y, w - 4, h, TFT_BLACK);

  int maxVal = 200;
  int levelClamped = soundLevel;
  if (levelClamped < 0) levelClamped = 0;
  if (levelClamped > maxVal) levelClamped = maxVal;

  int barW = (int)((float)levelClamped / maxVal * (w - 20));

  tft.drawRect(x + 10, y + 8, w - 20, 15, COLOR_LINE);
  tft.fillRect(x + 11, y + 9, barW - 2 < 0 ? 0 : barW - 2, 13, TFT_BLUE);

  tft.setTextColor(COLOR_VALUE, TFT_BLACK);
  tft.drawCentreString(String(levelClamped), x + w / 2, y + 30, 2);
}

void drawCurtainStatus(const String &curtainState) {
  int cellW = 480 / 4;
  int x = cellW * 3;
  int y = 255;
  int w = cellW;
  int h = 60;

  tft.fillRect(x + 2, y, w - 4, h, TFT_BLACK);

  uint16_t color;
  if (curtainState == "open") color = TFT_CYAN;
  else if (curtainState == "close") color = TFT_DARKGREY;
  else color = TFT_YELLOW;

  tft.fillRoundRect(x + 10, y + 5, w - 20, 20, 4, color);
  tft.setTextColor(TFT_BLACK, color);
  tft.drawCentreString(curtainState, x + w / 2, y + 8, 2);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();

  Serial.print("MQTT cmd topic = ");
  Serial.print(t);
  Serial.print("  payload = ");
  Serial.println(msg);

  if (t == String(mqttCmdFanTopic)) {
    if (msg.equalsIgnoreCase("on")) remoteFanMode = 1;
    else if (msg.equalsIgnoreCase("off")) remoteFanMode = 0;
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected!");
}

void connectMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);

  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("MKR1010_client", mqttUser, mqttPass)) {
      Serial.println(" connected!");
      mqttClient.subscribe(mqttCmdFanTopic);
      Serial.print("Subscribed to: ");
      Serial.println(mqttCmdFanTopic);
    } else {
      Serial.print(" failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  connectWiFi();
  mqttClient.setCallback(mqttCallback);
  connectMQTT();

  Serial.println("MLX90640 + SCD30 + Fan + Light + Servo + Flame + Buzzer + SoundLevel + MQTT + TFT (MKR1010)");

  tft.init();
  tft.setRotation(1);
  drawBaseLayout();

  Wire.begin();
  Wire.setClock(400000);

  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1);
  }

  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_4_HZ);

  scd30.begin();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  pinMode(SOUND_PIN, INPUT);

  curtainServo.attach(SERVO_PIN, 500, 2500);
  currentServoAngle = -1;

  baseline = readFastAverage(50);
  soundLevel = 0;
  lastSampleTime = millis();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = now;

    int current = readFastAverage(15);
    baseline = baselineAlpha * baseline + (1.0 - baselineAlpha) * current;
    int level = current - (int)baseline;
    if (level < 0) level = 0;

    soundLevel = (int)(soundAlpha * soundLevel + (1.0 - soundAlpha) * level);

    Serial.print("Sound level = ");
    Serial.println(soundLevel);
  }

  int status = mlx.getFrame(frame);
  if (status != 0) {
    delay(250);
    return;
  }

  bool human = detectHuman(frame);

  if (scd30.dataReady()) {
    scd30.read();
    latestCO2 = scd30.CO2;
    hasCO2 = true;
  }

  if (hasCO2) {
    Serial.print("CO2 = "); Serial.println(latestCO2);
    Serial.print("Temperature = "); Serial.println(scd30.temperature);
    Serial.print("Humidity = "); Serial.println(scd30.relative_humidity);
  }

  bool fanOn = false;
  String fanSource = "auto";

  if (human) {
    remoteFanMode = -1;
    if (hasCO2 && (latestCO2 > 5000.0 || scd30.temperature > 29.0)) {
      fanOn = true;
    }
  } else {
    if (remoteFanMode == 1) {
      fanOn = true;
      fanSource = "remote";
    } else if (remoteFanMode == 0) {
      fanOn = false;
      fanSource = "remote";
    }
  }

  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);
  Serial.print("Fan state: ");
  Serial.print(fanOn ? "ON" : "OFF");
  Serial.print("  source=");
  Serial.println(fanSource);

  int lightRaw = analogRead(LIGHT_PIN);
  Serial.print("Light raw = "); Serial.println(lightRaw);

  String curtainState = "";

  if (human) {
    if (lightRaw > LIGHT_THRESHOLD) {
      curtainState = "open";
      if (currentServoAngle != 180) {
        curtainServo.write(180);
        currentServoAngle = 180;
        Serial.println("Curtain: OPEN");
      }
    } else {
      curtainState = "close";
      if (currentServoAngle != 0) {
        curtainServo.write(0);
        currentServoAngle = 0;
        Serial.println("Curtain: CLOSE");
      }
    }
  } else {
    curtainState = "unchanged";
  }

  bool fireDetected = (digitalRead(FLAME_PIN) == HIGH);
  Serial.print("Fire: "); Serial.println(fireDetected ? "YES" : "NO");

  if (fireDetected && !lastFireDetected) {
    buzzerOn = true;
    buzzerStartMillis = millis();
    tone(BUZZER_PIN, 4000);
  }

  if (buzzerOn && (millis() - buzzerStartMillis >= 5000)) {
    buzzerOn = false;
    noTone(BUZZER_PIN);
  }

  lastFireDetected = fireDetected;

  updateMainDataPanel(human,
                      hasCO2,
                      latestCO2,
                      scd30.temperature,
                      scd30.relative_humidity,
                      lightRaw,
                      soundLevel);

  drawFanStatus(fanOn, fanSource);
  drawFireStatus(fireDetected);
  drawSoundStatus(soundLevel);
  drawCurtainStatus(curtainState);

  String payload = "{";
  payload += "\"presence\":" + String(human ? "true" : "false") + ",";
  payload += "\"co2\":" + String(latestCO2, 2) + ",";
  payload += "\"temperature\":" + String(scd30.temperature, 2) + ",";
  payload += "\"humidity\":" + String(scd30.relative_humidity, 2) + ",";
  payload += "\"fan\":" + String(fanOn ? "true" : "false") + ",";
  payload += "\"fan_source\":\"" + fanSource + "\",";
  payload += "\"light\":" + String(lightRaw) + ",";
  payload += "\"curtain\":\"" + curtainState + "\",";
  payload += "\"fire\":" + String(fireDetected ? "true" : "false") + ",";
  payload += "\"buzzer\":" + String(buzzerOn ? "true" : "false") + ",";
  payload += "\"sound_level\":" + String(soundLevel);
  payload += "}";

  mqttClient.publish(mqttTopic, payload.c_str());
  Serial.println("MQTT Sent:");
  Serial.println(payload);

  delay(250);
}
