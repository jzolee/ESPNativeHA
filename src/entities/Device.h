// =========================================================================
// File: src/entities/Device.h
// =========================================================================
#pragma once

#include <Arduino.h>
#include <map>
#include <ArduinoJson.h>
#include <vector>

class Device {
    friend class DeviceConfigurator;
    friend class MqttDriver;

private:
    String _unique_id;
    String _name;
    std::map<String, String> _attributes;
    std::vector<class BaseEntity*> _entities;

public:
    Device(const String& unique_id);
    const String& getUniqueId() const { return _unique_id; }
    const String& getName() const { return _name; }
    const std::map<String, String>& getAttributes() const { return _attributes; }
    const std::vector<class BaseEntity*>& getEntities() const { return _entities; }
    void addEntity(class BaseEntity* entity);
    void serializeToJson(JsonObject& doc) const;
};