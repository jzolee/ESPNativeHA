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

    // JAVÍTÁS: A begin() hívás most már NEM kér titkosítási kulcsot
    ha_client.begin(HA_DEVICE_NAME);
    
    Device& dev = ha_client.createDevice("my_native_device_01").setName("My Native Device");
    ha_client.createSensor(dev, "temperature", temperatureAgent).setName("Temperature");
}

void loop() {
    ha_client.loop();
}