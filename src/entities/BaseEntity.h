// =========================================================================
// File: src/entities/BaseEntity.h
// Leírás: Absztrakt ősosztály minden entitáshoz
// =========================================================================
#pragma once

#include <Arduino.h>
#include <map>
#include "Device.h"
#include "Agent.h"

class BaseEntity {
    friend class HAManager;
    friend class EntityConfigurator;
    friend class MqttDriver;

protected:
    String _unique_id;
    String _name;
    Device* _parent_device = nullptr;
    std::map<String, String> _attributes;
    bool _discovery_published = false;

public:
    virtual ~BaseEntity() {}

    const String& getUniqueId() const { return _unique_id; }
    const String& getName() const { return _name; }
    Device* getDevice() const { return _parent_device; }
    const std::map<String, String>& getAttributes() const { return _attributes; }

    bool isDiscoveryPublished() const { return _discovery_published; }
    void setDiscoveryPublished(bool state) { _discovery_published = state; }

    virtual const char* getComponentType() const = 0;
    virtual String getStateAsString() const = 0;
    virtual void attachToAgent(std::function<void()> on_change) = 0;

    virtual bool isCommandable() const { return false; }
    virtual void onCommand(const String& payload) {}
};