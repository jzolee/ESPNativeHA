// =========================================================================
//                             main.cpp
// =========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "ESPNativeHA.h"
#include "secrets.h"

const char* HA_DEVICE_NAME = "my-native-device";

// Létrehozzuk a központi vezérlő objektumot
ESPNativeHA ha_client;

// Agent-ek a szenzorokhoz
Agent<float> temperatureAgent(0.0f);
Agent<float> humidityAgent(0.0f);

void setup() {
    Serial.begin(115200);
    delay(1000);

    // WiFi csatlakozás
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    ha_client.setLogLevel(ESPNativeHA::LOG_LEVEL_DEBUG);
    ha_client.onConnect([]() { Serial.println(">>> Esemény: Home Assistant csatlakozott!"); });
    ha_client.onDisconnect([]() { Serial.println(">>> Esemény: Home Assistant lecsatlakozott!"); });

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();

    // JAVÍTÁS: A begin() hívás most már NEM kér titkosítási kulcsot
    ha_client.begin(HA_DEVICE_NAME, mac.c_str());

    Device& dev = ha_client.createDevice("my_native_device_01").setName("My Native Device")
        .setName("Időjárás Állomás")
        .setManufacturer("Zolee-Művek");

    ha_client.createSensor(dev, "temperature", temperatureAgent).setName("Temperature")
        .setName("Külső Hőmérséklet")
        .setIcon("mdi:thermometer")
        .setUnitOfMeasurement("°C");

    // Önálló szenzor létrehozása
    ha_client.createSensor("humidity_standalone", humidityAgent)
        .setName("Páratartalom")
        .setIcon("mdi:water-percent")
        .setUnitOfMeasurement("%");
}

void loop() {
    ha_client.loop();

    // Szimuláljuk a szenzorok frissülését 5 másodpercenként
    static unsigned long last_update = 0;
    if (millis() - last_update > 5000) {
        last_update = millis();
        float new_temp = random(150, 250) / 10.0;
        float new_hum = random(400, 600) / 10.0;

        Serial.printf("Updating sensor values: Temp=%.1f, Hum=%.1f\n", new_temp, new_hum);

        // Az Agent-ek értékének beállítása. A jövőben ez fogja triggerelni
        // az adatok elküldését a Home Assistant felé.
        temperatureAgent.set(new_temp);
        humidityAgent.set(new_hum);
    }
}