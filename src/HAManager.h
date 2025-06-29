// =========================================================================
// File: src/HAManager.h
// =========================================================================
#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>
#include "drivers/BaseDriver.h"
#include "entities/Device.h"
#include "entities/SpecializedEntities.h"
#include "entities/ComplexEntities.h"
#include "configurators/Configurator.h"

class HAManager {
    friend class MqttDriver;

private:
    BaseDriver* _driver = nullptr;
    std::vector<Device*> _devices;
    std::vector<BaseEntity*> _standalone_entities;

    std::function<void()> _onConnectCallback = nullptr;
    std::function<void()> _onDisconnectCallback = nullptr;
    std::function<void(const char*)> _onErrorCallback = nullptr;

public:
    HAManager();
    ~HAManager();

    void begin(BaseDriver* driver);
    void loop();
    
    void onConnect(const std::function<void()>& callback);
    void onDisconnect(const std::function<void()>& callback);
    void onError(const std::function<void(const char*)>& callback);

    void fireConnectEvent();
    void fireDisconnectEvent();
    void fireErrorEvent(const char* error);

    const std::vector<Device*>& getDevices() const { return _devices; }
    const std::vector<BaseEntity*>& getStandaloneEntities() const { return _standalone_entities; }

    DeviceConfigurator createDevice(const String& unique_id);

    template<typename T>
    EntityConfigurator createSensor(const String& unique_id, Agent<T>& agent, Device* device = nullptr) {
        return addEntity(new Sensor<T>(unique_id, agent), device);
    }
    EntityConfigurator createBinarySensor(const String& unique_id, Agent<bool>& agent, Device* device = nullptr) {
        return addEntity(new BinarySensor(unique_id, agent), device);
    }
    EntityConfigurator createSwitch(const String& unique_id, Agent<bool>& agent, Device* device = nullptr) {
        return addEntity(new Switch(unique_id, agent), device);
    }
    template<typename T>
    EntityConfigurator createNumber(const String& unique_id, Agent<T>& agent, Device* device = nullptr) {
        return addEntity(new Number<T>(unique_id, agent), device);
    }
    EntityConfigurator createSelect(const String& unique_id, Agent<String>& agent, Device* device = nullptr) {
        return addEntity(new Select(unique_id, agent), device);
    }
    EntityConfigurator createButton(const String& unique_id, Agent<bool>& agent, Device* device = nullptr) {
        return addEntity(new Button(unique_id, agent), device);
    }
    EntityConfigurator createText(const String& unique_id, Agent<String>& agent, Device* device = nullptr) {
        return addEntity(new Text(unique_id, agent), device);
    }
    EntityConfigurator createLock(const String& unique_id, Agent<bool>& agent, Device* device = nullptr) {
        return addEntity(new Lock(unique_id, agent), device);
    }
    EntityConfigurator createCover(const String& unique_id, Agent<String>& agent, Device* device = nullptr) {
        return addEntity(new Cover(unique_id, agent), device);
    }
    EntityConfigurator createLight(const String& unique_id, Agent<bool>& state, Agent<int>& brightness, Device* device = nullptr) {
        return addEntity(new Light(unique_id, state, brightness), device);
    }
    EntityConfigurator createSiren(const String& unique_id, Agent<bool>& agent, Device* device = nullptr) {
        return addEntity(new Siren(unique_id, agent), device);
    }
    
private:
    EntityConfigurator addEntity(BaseEntity* entity, Device* device);
};