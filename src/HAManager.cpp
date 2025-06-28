// =========================================================================
// File: src/HAManager.cpp
// Leírás: A HAManager implementációja
// =========================================================================
#include "HAManager.h"

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
        Serial.println("FATAL: No driver set for HAManager!");
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