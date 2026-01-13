#include <Wire.h>
#include <Adafruit_MLX90640.h>
#include <Adafruit_SCD30.h>
#include <Servo.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

char ssid[] = "";
char pass[] = "";

const char* mqttServer = "mqtt.cetools.org";
const int   mqttPort   = 1884;
const char* mqttUser   = "student";
const char* mqttPass   = "ce2021-mqtt-forget-whale";
const char* mqttTopic  = "student/MUJI/HZH";

const char* mqttCmdFanTopic = "student/MUJI/HZH/command/fan";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Adafruit_MLX90640 mlx;
Adafruit_SCD30  scd30;
Servo curtainServo;

float frame[32 * 24];

const float TEMP_DELTA_THRESHOLD = 3.5;
const int   MIN_HOT_PIXELS       = 8;

const int FAN_PIN = 7;

const int LIGHT_PIN       = A1;
const int LIGHT_THRESHOLD = 600;
const int SERVO_PIN       = 6;

const int FLAME_PIN = 2;

const int BUZZER_PIN = 5;

float latestCO2 = NAN;
bool  hasCO2    = false;

int currentServoAngle = -1;

bool buzzerOn = false;
unsigned long buzzerStartMillis = 0;
bool lastFireDetected = false;

int remoteFanMode = -1;

bool detectHuman(const float *f) {
  float sum  = 0.0;
  float minT = 1000.0;
  float maxT = -1000.0;

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

  bool presence = (hotPixels >= MIN_HOT_PIXELS) &&
                  ((maxT - avg) > TEMP_DELTA_THRESHOLD);

  Serial.print("IR avg="); Serial.print(avg, 2);
  Serial.print("  max="); Serial.print(maxT, 2);
  Serial.print("  hotPixels="); Serial.print(hotPixels);
  Serial.print("  presence=");
  Serial.println(presence ? "YES" : "NO");

  return presence;
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
    if (msg.equalsIgnoreCase("on")) {
      remoteFanMode = 1;
      Serial.println("Remote fan command: ON");
    } else if (msg.equalsIgnoreCase("off")) {
      remoteFanMode = 0;
      Serial.println("Remote fan command: OFF");
    }
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

  Serial.println("MLX90640 + SCD30 + Fan + Light + Servo + Flame + Buzzer(passive) + MQTT (MKR1010)");

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

  curtainServo.attach(SERVO_PIN, 500, 2500);
  currentServoAngle = -1;
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

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
    fanSource = "auto";
  } else {
    if (remoteFanMode == 1) {
      fanOn = true;
      fanSource = "remote";
    } else if (remoteFanMode == 0) {
      fanOn = false;
      fanSource = "remote";
    } else {
      fanOn = false;
      fanSource = "auto";
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
        Serial.println("Curtain: OPEN (human & light OK)");
      }
    } else {
      curtainState = "close";
      if (currentServoAngle != 0) {
        curtainServo.write(0);
        currentServoAngle = 0;
        Serial.println("Curtain: CLOSE (human & light LOW)");
      }
    }
  } else {
    curtainState = "unchanged";
    Serial.println("No human: curtain stays.");
  }

  bool fireDetected = (digitalRead(FLAME_PIN) == HIGH);
  Serial.print("Fire: "); Serial.println(fireDetected ? "YES" : "NO");

  if (fireDetected && !lastFireDetected) {
    buzzerOn = true;
    buzzerStartMillis = millis();
    tone(BUZZER_PIN, 4000);
    Serial.println("Buzzer: ON (fire detected, start 5s alarm)");
  }

  if (buzzerOn && (millis() - buzzerStartMillis >= 5000)) {
    buzzerOn = false;
    noTone(BUZZER_PIN);
    Serial.println("Buzzer: OFF (5s alarm finished)");
  }

  lastFireDetected = fireDetected;

  Serial.println("---------------------");

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
  payload += "\"buzzer\":" + String(buzzerOn ? "true" : "false");
  payload += "}";

  mqttClient.publish(mqttTopic, payload.c_str());
  Serial.println("MQTT Sent:");
  Serial.println(payload);

  delay(250);
}
