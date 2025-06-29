// =========================================================================
// File: src/HAManager.cpp
// =========================================================================
#include "HAManager.h"
#include "core/Logger.h"

HAManager::HAManager() {}

HAManager::~HAManager() {
    for (auto p : _devices) delete p;
    _devices.clear();
    for (auto p : _standalone_entities) delete p;
    _standalone_entities.clear();
}

void HAManager::begin(BaseDriver* driver) {
    _driver = driver;
    if (!_driver) {
        ha_log->error("HAManager: No driver set!");
        return;
    }
    for(auto* dev : _devices) {
        for(auto* ent : dev->getEntities()){
            _driver->registerEntity(ent);
        }
    }
    for(auto* ent : _standalone_entities) {
        _driver->registerEntity(ent);
    }
    _driver->begin(this);
}

void HAManager::loop() {
    if (_driver) {
        _driver->loop();
    }
}

DeviceConfigurator HAManager::createDevice(const String& unique_id) {
    auto* dev = new Device(unique_id);
    _devices.push_back(dev);
    return DeviceConfigurator(*dev);
}

EntityConfigurator HAManager::addEntity(BaseEntity* entity, Device* device) {
    if (device) {
        entity->_parent_device = device;
        device->addEntity(entity);
    } else {
        _standalone_entities.push_back(entity);
    }
    if (_driver) {
        _driver->registerEntity(entity);
    }
    return EntityConfigurator(*entity);
}

void HAManager::onConnect(const std::function<void()>& callback) { _onConnectCallback = callback; }
void HAManager::onDisconnect(const std::function<void()>& callback) { _onDisconnectCallback = callback; }
void HAManager::onError(const std::function<void(const char*)>& callback) { _onErrorCallback = callback; }
void HAManager::fireConnectEvent() { if (_onConnectCallback) _onConnectCallback(); }
void HAManager::fireDisconnectEvent() { if (_onDisconnectCallback) _onDisconnectCallback(); }
void HAManager::fireErrorEvent(const char* error) { if (_onErrorCallback) _onErrorCallback(error); }
