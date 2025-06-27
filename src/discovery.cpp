// =========================================================================
//                         src/discovery.cpp
// =========================================================================
#include "discovery.h"

DiscoveryHandler::DiscoveryHandler() {}

void DiscoveryHandler::begin(const String& device_name) {
    _device_name = "esphome-dev2v";
    //_device_name += device_name;
    //_device_name += ".local";
    MDNS.begin(_device_name);
    _udp.listen(5353);
}
void DiscoveryHandler::loop() {
    if (millis() - _last_broadcast_time > BROADCAST_INTERVAL) {
        _last_broadcast_time = millis();
        _sendUDPPacket();
    }
}

void DiscoveryHandler::_sendUDPPacket() {
    String message = R"({"name":"esphome-dev2v","platform":"esp8266","board":"esp01"})";
    _udp.broadcastTo(message.c_str(), 5353);
}