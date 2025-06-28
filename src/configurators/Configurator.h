// =========================================================================
// File: src/configurators/Configurator.h
// Leírás: Builder minta a Device és Entity objektumok elegáns létrehozásához.
// =========================================================================
#pragma once

#include "../entities/BaseEntity.h"
#include "../entities/Device.h"

class EntityConfigurator {
private:
    BaseEntity& _entity;
public:
    EntityConfigurator(BaseEntity& entity) : _entity(entity) {}
    EntityConfigurator& setName(const String& name) { _entity._name = name; return *this; }
    EntityConfigurator& setAttribute(const String& key, const String& value) { _entity._attributes[key] = value; return *this; }
    EntityConfigurator& setIcon(const String& icon) { return setAttribute("icon", icon); }
};

class DeviceConfigurator {
private:
    Device& _device;
public:
    DeviceConfigurator(Device& device) : _device(device) {}
    DeviceConfigurator& setName(const String& name) { _device._name = name; return *this; }
    DeviceConfigurator& setAttribute(const String& key, const String& value) { _device._attributes[key] = value; return *this; }
    DeviceConfigurator& setManufacturer(const String& man) { return setAttribute("manufacturer", man); }
    DeviceConfigurator& setModel(const String& model) { return setAttribute("model", model); }
    DeviceConfigurator& setSoftwareVersion(const String& sw) { return setAttribute("sw_version", sw); }
    operator Device&() { return _device; }
};