// =========================================================================
//                             src/ESPNativeHA.cpp
// =========================================================================
#include "ESPNativeHA.h"
#include <cstdarg>
#include <WiFi.h>

ESPNativeHA::ESPNativeHA() {
    _comm_handler.on_connect = [this]() {
        if (this->_on_connect_callback) this->_on_connect_callback();
    };
    _comm_handler.on_disconnect = [this]() {
        if (this->_on_disconnect_callback) this->_on_disconnect_callback();
    };
    _comm_handler.on_error = [this](const std::string &error) {
        this->_log_printf(LOG_LEVEL_ERROR, "Communication Error: %s", error.c_str());
    };
    _comm_handler.on_hello_response = [this](const HelloResponse& response){
        this->_log_printf(LOG_LEVEL_INFO, "Received HelloResponse from server: %s (API v%d.%d)", response.server_info, response.api_version_major, response.api_version_minor);
    };
}

ESPNativeHA::~ESPNativeHA() {
    for (auto p : _devices) delete p;
    for (auto p : _entities) delete p;
}

void ESPNativeHA::begin(const char* device_name, const char* project_name, const char* project_version, uint16_t port) {
    _log_printf(LOG_LEVEL_INFO, "ESPNativeHA starting up for device: %s", device_name);
    
    _comm_handler.begin(device_name, port);
    _discovery_handler.begin(device_name);
}

void ESPNativeHA::loop() {
    _comm_handler.loop();
    _discovery_handler.loop();
}

void ESPNativeHA::onConnect(std::function<void()> callback) { this->_on_connect_callback = callback; }
void ESPNativeHA::onDisconnect(std::function<void()> callback) { this->_on_disconnect_callback = callback; }
void ESPNativeHA::setLogLevel(LogLevel level) { _logLevel = level; }

DeviceConfigurator ESPNativeHA::createDevice(const char* unique_id) { 
    Device* newDevice = new Device();
    newDevice->unique_id = unique_id;
    _devices.push_back(newDevice);
    return DeviceConfigurator(*newDevice);
}
EntityConfigurator ESPNativeHA::createSensor(Device& device, const char* entity_id, Agent<float>& agent) { 
    FloatSensorEntity* newSensor = new FloatSensorEntity(agent);
    newSensor->entity_id = entity_id;
    newSensor->parent_device = &device;
    _entities.push_back(newSensor);
    return EntityConfigurator(*newSensor);
}
EntityConfigurator ESPNativeHA::createSensor(const char* entity_id, Agent<float>& agent) { 
    FloatSensorEntity* newSensor = new FloatSensorEntity(agent);
    newSensor->entity_id = entity_id;
    newSensor->parent_device = nullptr;
    _entities.push_back(newSensor);
    return EntityConfigurator(*newSensor);
}

void ESPNativeHA::_log_printf(LogLevel level, const char* format, ...) {
    if (_logLevel >= level) {
        static char buf[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        Serial.printf("[ESPNativeHA] %s\n", buf);
    }
}