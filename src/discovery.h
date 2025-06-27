// =========================================================================
//                         src/discovery.h
// =========================================================================
#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>

class DiscoveryHandler {
public:
    DiscoveryHandler();
    // JAVÍTÁS: A begin most már a MAC címet is kéri a TXT rekordhoz
    void begin(const String& device_name, const String& project_name, const String& project_version, uint16_t service_port);
    void loop();
private:
    String _device_name;
    uint16_t _service_port;
    WiFiUDP _udp;
    unsigned long _last_broadcast_time = 0;
    const unsigned long BROADCAST_INTERVAL = 30000;
    void _sendUDPPacket();
};

#endif // DISCOVERY_H
