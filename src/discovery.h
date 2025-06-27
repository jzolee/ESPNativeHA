// =========================================================================
//                         src/discovery.h
// =========================================================================
#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <Arduino.h>
#ifdef ESP32
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESP8266mDNS.h>
#endif
#include <AsyncUDP.h>  // ESP32-ben beépített, ESP8266-hoz telepíteni kell

class DiscoveryHandler {
public:
    DiscoveryHandler();
    void begin(const String& device_name);
    void loop();
private:
    String _device_name;
    AsyncUDP _udp;
    unsigned long _last_broadcast_time = 0;
    const unsigned long BROADCAST_INTERVAL = 30000;
    void _sendUDPPacket();
};

#endif // DISCOVERY_H
