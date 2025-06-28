// =========================================================================
//                         src/communication.h
// =========================================================================
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <functional>
#include <vector>
#include <string>
#include "api.pb.h" // A Nanopb által generált header

class CommunicationHandler;

class ConnectionSession {
public:
    ConnectionSession(AsyncClient* client, CommunicationHandler* parent);
    ~ConnectionSession();
    void loop();
private:
    void onData(uint8_t* data, size_t len);
    void handlePacket(uint8_t* data, size_t len);
    void sendHelloRequest();
    void sendPacket(const uint8_t* data, size_t len);
    size_t encodeVarint(uint8_t* buffer, uint32_t value);
    
    enum ReadState { READ_PREAMBLE, READ_LENGTH, READ_PACKET };
    
    AsyncClient* _client;
    CommunicationHandler* _parent;
    bool _needs_to_send_hello = true;
    ReadState _read_state = READ_PREAMBLE;
    std::vector<uint8_t> _read_buffer;
    uint32_t _packet_len = 0;
    uint32_t _packet_type = 0;
};

class CommunicationHandler {
    friend class ConnectionSession; 
public:
    CommunicationHandler();
    ~CommunicationHandler();
    void begin(const String& client_info);
    void loop();
    std::function<void()> on_connect;
    std::function<void()> on_disconnect;
    std::function<void(const std::string &)> on_error;
    std::function<void(const HelloResponse&)> on_hello_response;
private:
    void _onClientConnect(void* arg, AsyncClient* client);
    String _client_info;
    AsyncServer* _server = nullptr;
    ConnectionSession* _connection = nullptr;
};

#endif // COMMUNICATION_H
