// =========================================================================
//                         src/discovery.cpp
// =========================================================================
#include "discovery.h"
#include <WiFi.h>

const int UDP_BROADCAST_PORT = 3232;

DiscoveryHandler::DiscoveryHandler() {}

void DiscoveryHandler::begin(const String& device_name, const String& project_name, const String& project_version, uint16_t service_port) {
    _device_name = device_name;
    _service_port = service_port;
    
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();
    
    if (MDNS.begin(_device_name.c_str())) {
        MDNS.addService("esphome", "tcp", _service_port);
        // JAVÍTÁS: A teljes, hivatalos TXT rekordok hozzáadása
        MDNS.addServiceTxt("esphome", "tcp", "version", "2.0.0"); // A mi API verziónk
        MDNS.addServiceTxt("esphome", "tcp", "platform", "ESP32");
        MDNS.addServiceTxt("esphome", "tcp", "mac", mac);
        MDNS.addServiceTxt("esphome", "tcp", "friendly_name", _device_name);
        MDNS.addServiceTxt("esphome", "tcp", "project_name", project_name);
        MDNS.addServiceTxt("esphome", "tcp", "project_version", project_version);
    }
    
    _udp.begin(UDP_BROADCAST_PORT);
}

void DiscoveryHandler::loop() {
    if (millis() - _last_broadcast_time > BROADCAST_INTERVAL) {
        _last_broadcast_time = millis();
        _sendUDPPacket();
    }
}

void DiscoveryHandler::_sendUDPPacket() {
    String message = "esphome-broadcast:name=" + _device_name + ";address=" + WiFi.localIP().toString() + ";port=" + String(_service_port) + ";password_protected=0";
    IPAddress broadcastIp(255, 255, 255, 255);
    _udp.beginPacket(broadcastIp, UDP_BROADCAST_PORT);
    _udp.print(message);
    _udp.endPacket();
}