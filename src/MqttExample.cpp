// =========================================================================
// File: MqttExample.ino (a fő projekt mappában)
// Leírás: Frissített példaprogram az AsyncMqttDriver használatával.
// =========================================================================
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include "HAManager.h"
#include "drivers/MqttDriver.h"
#include "Agent.h"

#include "secrets.h"

// --- Globális Objektumok ---
HAManager haManager;
MqttDriver mqttDriver(MQTT_SERVER, MQTT_PORT);

// --- Agent-ek az adatok tárolására ---

// Konyhai eszközök agentjei
Agent<float> kitchenTemperatureAgent(22.5f);
Agent<bool> kitchenLightAgent(false);

// Nappali eszközök agentjei
Agent<int> livingRoomBrightnessAgent(128);
Agent<bool> tvPowerAgent(true);

// Önálló (standalone) agentek
Agent<int> uptimeAgent(0);
Agent<bool> systemStatusAgent(true); // Pl. egy általános "OK" jelzés

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nBooting complex example with multiple devices...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    mqttDriver.setUser(MQTT_USER, MQTT_PASSWORD);

    // ====================== ESZKÖZ 1: KONYHA ======================
    Device& kitchenDevice = haManager.createDevice("konyhai_eszkozok_01")
        .setName("Konyhai Eszközök")
        .setManufacturer("Zolee-Művek")
        .setModel("Konyha-ESP-v1");

    haManager.createEntity<float>("konyha_hom", "sensor", kitchenTemperatureAgent, &kitchenDevice)
        .setName("Konyha Hőmérséklet")
        .setAttribute("device_class", "temperature")
        .setAttribute("unit_of_measurement", "°C");

    haManager.createEntity<bool>("konyha_lampa", "switch", kitchenLightAgent, &kitchenDevice)
        .setName("Konyha Lámpa")
        .setIcon("mdi:ceiling-light");

    // ====================== ESZKÖZ 2: NAPPALI ======================
    Device& livingRoomDevice = haManager.createDevice("nappali_media_01")
        .setName("Nappali Média Center")
        .setManufacturer("Zolee-Művek")
        .setModel("Média-ESP-v2");

    haManager.createEntity<int>("nappali_fenyero", "number", livingRoomBrightnessAgent, &livingRoomDevice)
        .setName("Nappali Hangulatfény")
        .setIcon("mdi:brightness-6")
        .setAttribute("min", "0")
        .setAttribute("max", "255")
        .setAttribute("mode", "slider");

    haManager.createEntity<bool>("nappali_tv", "switch", tvPowerAgent, &livingRoomDevice)
        .setName("TV Táp")
        .setIcon("mdi:television");


    // ====================== ÖNÁLLÓ ENTITÁSOK ======================
    haManager.createEntity<int>("esp_uptime", "sensor", uptimeAgent)
        .setName("ESP Uptime")
        .setIcon("mdi:timer-sand")
        .setAttribute("state_class", "total_increasing")
        .setAttribute("unit_of_measurement", "s");
    
    haManager.createEntity<bool>("esp_system_ok", "binary_sensor", systemStatusAgent)
        .setName("ESP Rendszer Állapot")
        .setAttribute("device_class", "problem")
        .setAttribute("payload_on", "OFF")  // Fordított logika: a "problem" device class akkor "OK", ha az állapot "OFF"
        .setAttribute("payload_off", "ON");


    // ====================== INDÍTÁS ======================
    haManager.begin(&mqttDriver);

    Serial.println("Setup complete.");

    // --- Visszacsatolások beállítása ---
    kitchenLightAgent.attach([](bool state){ Serial.printf(">>> Visszajelzés: Konyha lámpa -> %s\n", state ? "ON" : "OFF"); });
    tvPowerAgent.attach([](bool state){ Serial.printf(">>> Visszajelzés: TV Táp -> %s\n", state ? "ON" : "OFF"); });
    livingRoomBrightnessAgent.attach([](int value){ Serial.printf(">>> Visszajelzés: Nappali fényerő -> %d\n", value); });
}

void loop() {
    haManager.loop();

    // Szimulált szenzor adatok és állapotváltozások
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 15000) { // 15 másodpercenként
        lastUpdate = millis();
        kitchenTemperatureAgent.set(random(190, 230) / 10.0f);
        uptimeAgent.set(millis() / 1000);
    }
}