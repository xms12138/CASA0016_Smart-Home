#include <Wire.h>
#include <Adafruit_SCD30.h>

Adafruit_SCD30 scd30;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing SCD30...");

  if (!scd30.begin()) {
    Serial.println("❌ SCD30 not found! Check wiring.");
    while (1) delay(10);
  }

  Serial.println("✅ SCD30 found!");
}

void loop() {
  if (scd30.read()) {
    Serial.print("CO2: ");
    Serial.print(scd30.CO2);
    Serial.println(" ppm");

    Serial.print("Temperature: ");
    Serial.print(scd30.temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(scd30.relative_humidity);
    Serial.println(" %");

    Serial.println("---------------------");
  } else {
    Serial.println("Waiting for measurement...");
  }

  delay(2000);  
}
