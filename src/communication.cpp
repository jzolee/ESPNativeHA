// =========================================================================
//                         src/communication.cpp
// =========================================================================
#include "communication.h"
#include <pb_encode.h>
#include <pb_decode.h>

// ---- CommunicationHandler Implementáció ----

CommunicationHandler::CommunicationHandler() {}

CommunicationHandler::~CommunicationHandler() {
    if (_server) { _server->end(); delete _server; }
    if (_connection) { delete _connection; }
}

void CommunicationHandler::begin(const String& client_info, uint16_t port) {
    _client_info = client_info;
    _server = new AsyncServer(port);
    _server->onClient([this](void* arg, AsyncClient* client) {
        this->_onClientConnect(arg, client);
    }, this);
    _server->begin();
}

void CommunicationHandler::loop() {
    if (_connection) {
        _connection->loop();
    }
}

void CommunicationHandler::_onClientConnect(void* arg, AsyncClient* client) {
    if (_connection != nullptr) {
        client->stop();
        return;
    }
    _connection = new ConnectionSession(client, this);
    if (on_connect) on_connect();
    client->onDisconnect([this](void*, AsyncClient*) {
        if (this->on_disconnect) this->on_disconnect();
        if (this->_connection) { delete this->_connection; this->_connection = nullptr; }
    }, this);
}

// ---- ConnectionSession Implementáció ----

ConnectionSession::ConnectionSession(AsyncClient* client, CommunicationHandler* parent) 
    : _client(client), _parent(parent) {
    _client->onData([this](void*, AsyncClient*, void* data, size_t len){
        this->onData((uint8_t*)data, len);
    }, this);
}

ConnectionSession::~ConnectionSession() {}

void ConnectionSession::loop() {
    if (_needs_to_send_hello) {
        sendHelloRequest();
        _needs_to_send_hello = false;
    }
}

void ConnectionSession::onData(uint8_t* data, size_t len) {
    _read_buffer.insert(_read_buffer.end(), data, data + len);

    while (!_read_buffer.empty()) {
        if (_read_state == READ_PREAMBLE) {
            if (_read_buffer[0] != 0x00) {
                if (_parent->on_error) _parent->on_error("Invalid preamble from server.");
                _client->close(true);
                return;
            }
            _read_buffer.erase(_read_buffer.begin());
            _read_state = READ_LENGTH;
        }
        
        if (_read_state == READ_LENGTH) {
            pb_istream_t stream = pb_istream_from_buffer(_read_buffer.data(), _read_buffer.size());
            const size_t initial_size = _read_buffer.size();

            if (!pb_decode_varint32(&stream, &_packet_len)) return; 
            if (!pb_decode_varint32(&stream, &_packet_type)) return;

            const size_t header_len = initial_size - stream.bytes_left;
            _read_buffer.erase(_read_buffer.begin(), _read_buffer.begin() + header_len);
            
            _read_state = READ_PACKET;
        }

        if (_read_state == READ_PACKET) {
            if (_read_buffer.size() < _packet_len) return;
            
            handlePacket(_read_buffer.data(), _packet_len);
            
            _read_buffer.erase(_read_buffer.begin(), _read_buffer.begin() + _packet_len);
            _read_state = READ_PREAMBLE; 
        }
    }
}

void ConnectionSession::handlePacket(uint8_t* data, size_t len) {
    if (_packet_type == 1) { // HelloResponse
        HelloResponse message = HelloResponse_init_zero;
        pb_istream_t stream = pb_istream_from_buffer(data, len);
        if (!pb_decode(&stream, HelloResponse_fields, &message)) {
            if (_parent->on_error) _parent->on_error("Failed to decode HelloResponse.");
            return;
        }
        if (_parent->on_hello_response) _parent->on_hello_response(message);
    }
}

void ConnectionSession::sendHelloRequest() {
    HelloRequest message = HelloRequest_init_zero;
    strncpy(message.client_id, _parent->_client_info.c_str(), sizeof(message.client_id) - 1);
    message.api_version_major = 1;
    message.api_version_minor = 1;
    uint8_t buffer[HelloRequest_size];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&stream, HelloRequest_fields, &message)) return;
    sendPacket(buffer, stream.bytes_written);
}

void ConnectionSession::sendPacket(const uint8_t* data, size_t len) {
    if (!_client->canSend()) return;
    
    uint8_t preamble = 0x00;
    _client->add((char*)&preamble, 1);
    
    uint8_t len_buffer[10];
    pb_ostream_t ostream = pb_ostream_from_buffer(len_buffer, sizeof(len_buffer));
    pb_encode_varint(&ostream, len + 1); // +1 a típus bájtért
    pb_encode_varint(&ostream, 0);       // HelloRequest type = 0
    
    _client->add((char*)len_buffer, ostream.bytes_written);
    _client->add((char*)data, len);
    _client->send();
}

size_t ConnectionSession::encodeVarint(uint8_t* buffer, uint32_t value) {
    size_t i = 0;
    while (value >= 0x80) { buffer[i++] = (value & 0x7F) | 0x80; value >>= 7; }
    buffer[i++] = value;
    return i;
}