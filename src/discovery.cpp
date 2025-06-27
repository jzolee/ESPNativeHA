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
    _udp.listen(6053);
}
void DiscoveryHandler::loop() {
    if (millis() - _last_broadcast_time > BROADCAST_INTERVAL) {
        _last_broadcast_time = millis();
        _sendUDPPacket();
    }
}

void DiscoveryHandler::_sendUDPPacket() {
    _udp.broadcastTo(_device_name.c_str(), 6053);
}