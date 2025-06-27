// =========================================================================
//                             main.cpp
// =========================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "ESPNativeHA.h"

// -- WiFi & HA Beállítások --
const char* WIFI_SSID = "Zx";
const char* WIFI_PASSWORD = "AbCdEfGhIjKlMnOp";
//const char* HA_IP_ADDRESS = "192.168.0.103";
// JAVÍTÁS: A működő eszközről kimásolt NÉV és KULCS helye
const char* HA_DEVICE_NAME = "my-native-device";
//const char* HA_ENCRYPTION_KEY = "sSlmHaX1ibeWHnSnOXHnq4B9SsN0RrhAzoG3spq85Zc=";


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