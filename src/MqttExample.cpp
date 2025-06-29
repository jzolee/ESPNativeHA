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
#include "core/Logger.h" // Logger beemelése

#include "secrets.h"

// --- Globális Objektumok ---
HAManager haManager;
MqttDriver mqttDriver(MQTT_SERVER, MQTT_PORT);

// Agent-ek az adatok tárolására
Agent<bool> officeLightStateAgent(false);
Agent<int> officeLightBrightnessAgent(200);
Agent<bool> officeLockStateAgent(true); 
Agent<String> livingRoomCoverStateAgent("closed");
Agent<bool> customSirenAgent(false);
Agent<int> uptimeAgent(0);
Agent<bool> restartButtonAgent(false);

void setup() {
    Serial.begin(115200);
    delay(1000);

    ha_log->setLogLevel(LogLevel::DEBUG);
    ha_log->onLog([](LogLevel level, const char* message){
        const char* levelStr = "";
        switch(level) {
            case LogLevel::ERROR: levelStr = "[E] "; break;
            case LogLevel::WARN:  levelStr = "[W] "; break;
            case LogLevel::INFO:  levelStr = "[I] "; break;
            case LogLevel::DEBUG: levelStr = "[D] "; break;
            default: break;
        }
        Serial.print(levelStr);
        Serial.println(message);
    });

    ha_log->info("Booting Showcase Example...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        ha_log->info(".");
    }
    ha_log->info("WiFi connected!");

    mqttDriver.setUser(MQTT_USER, MQTT_PASSWORD);

    // ====================== ESZKÖZ 1: IRODA ======================
    Device& officeDevice = haManager.createDevice("irodai_eszkoz_02")
        .setName("Irodai Vezérlő")
        .setManufacturer("Zolee-Művek");

    haManager.createLight("iroda_mennyezeti_lampa", officeLightStateAgent, officeLightBrightnessAgent, &officeDevice)
        .setName("Irodai Lámpa")
        .setIcon("mdi:ceiling-light-multiple");

    haManager.createLock("iroda_ajto_zar", officeLockStateAgent, &officeDevice)
        .setName("Iroda Ajtózár")
        .setIcon("mdi:lock");

    // ====================== ESZKÖZ 2: NAPPALI ======================
    Device& livingRoomDevice = haManager.createDevice("nappali_vezerlo_01")
        .setName("Nappali Vezérlő");

    haManager.createCover("nappali_redony", livingRoomCoverStateAgent, &livingRoomDevice)
        .setName("Nappali Redőny")
        .setAttribute("device_class", "shutter");
    
    haManager.createSiren("custom_siren", customSirenAgent, &livingRoomDevice)
        .setName("Riasztó Sziréna")
        .setIcon("mdi:alarm-bell");
        
    // ====================== ÖNÁLLÓ ENTITÁSOK ======================
    haManager.createSensor<int>("esp_uptime", uptimeAgent)
        .setName("ESP Uptime")
        .setIcon("mdi:timer-sand")
        .setAttribute("state_class", "total_increasing")
        .setAttribute("unit_of_measurement", "s");

    haManager.createButton("esp_reboot", restartButtonAgent)
        .setName("ESP32 Újraindítása")
        .setIcon("mdi:restart-alert");

    // ====================== INDÍTÁS ======================
    haManager.begin(&mqttDriver);
    ha_log->info("Setup complete.");

    // --- Visszacsatolások beállítása ---
    officeLockStateAgent.attach([](bool state){ ha_log->info(">>> Callback: Iroda zár új állapota: %s", state ? "LOCKED" : "UNLOCKED"); });
    livingRoomCoverStateAgent.attach([](String state){ ha_log->info(">>> Callback: Nappali redőny parancs: %s", state.c_str()); });
    officeLightStateAgent.attach([](bool state){ ha_log->info(">>> Callback: Lámpa állapot -> %s", state ? "ON" : "OFF"); });
    officeLightBrightnessAgent.attach([](int brightness){ ha_log->info(">>> Callback: Lámpa fényerő -> %d", brightness); });
    customSirenAgent.attach([](bool state) {
        ha_log->warn(">>> EGYÉNI KEZELÉS: A sziréna új állapota: %s", state ? "ACTIVE" : "INACTIVE");
    });
    restartButtonAgent.attach([](bool val) {
        ha_log->warn(">>> GOMB MEGNYOMVA: Eszköz újraindítása 3 másodperc múlva!");
        delay(3000);
        ESP.restart();
    });
}

void loop() {
    haManager.loop();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 30000) {
        lastUpdate = millis();
        ha_log->info("--- Szimuláció: Uptime frissítése ---");
        uptimeAgent.set(millis() / 1000);
    }
}