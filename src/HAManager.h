// =========================================================================
// File: src/HAManager.h
// Leírás: A központi "karmester" osztály
// =========================================================================
#pragma once

#include <Arduino.h>
#include <vector>
#include "drivers/BaseDriver.h"
#include "entities/Device.h"
#include "entities/TypedEntity.h"
#include "configurators/Configurator.h"

class HAManager {
private:
    BaseDriver* _driver = nullptr;
    std::vector<Device*> _devices;
    std::vector<BaseEntity*> _standalone_entities;

public:
    HAManager();
    ~HAManager();

    void begin(BaseDriver* driver);
    void loop();
    
    const std::vector<Device*>& getDevices() const { return _devices; }
    const std::vector<BaseEntity*>& getStandaloneEntities() const { return _standalone_entities; }

    DeviceConfigurator createDevice(const String& unique_id);

    template <typename T>
    EntityConfigurator createEntity(const String& unique_id, const char* component_type, Agent<T>& agent, Device* device = nullptr) {
        auto* entity = new TypedEntity<T>(unique_id, component_type, agent);
        entity->_name = unique_id;
        
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
};