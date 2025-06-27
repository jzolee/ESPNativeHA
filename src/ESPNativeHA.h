#ifndef ESP_NATIVE_HA_H
#define ESP_NATIVE_HA_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <map>
#include <Agent.h>
#include "communication.h"
#include "discovery.h"

// Előre deklarációk
class ESPNativeHA;
class Device;
class Entity;
class EntityConfigurator;
class DeviceConfigurator;

// --- Adatszerkezetek ---

class Device {
    friend class DeviceConfigurator;
    friend class ESPNativeHA;
protected:
    String unique_id;
    String name;
    std::map<String, String> attributes;
};

class Entity {
    friend class EntityConfigurator;
    friend class ESPNativeHA;
protected:
    String entity_id;
    String name;
    Device* parent_device = nullptr;
    std::map<String, String> attributes;

public:
    virtual ~Entity() {}
    virtual const char* getEntityType() = 0;
};

class FloatSensorEntity : public Entity {
    friend class ESPNativeHA;
protected:
    Agent<float>& _agent;
public:
    FloatSensorEntity(Agent<float>& agent) : _agent(agent) {}
    const char* getEntityType() override { return "sensor"; }
};

// --- Konfigurátorok (Builder Pattern) ---

class EntityConfigurator {
public:
    EntityConfigurator(Entity& entity) : _entity(entity) {}
    EntityConfigurator& setName(const char* name) { _entity.name = name; return *this; }
    EntityConfigurator& setIcon(const char* icon) { return setAttribute("icon", icon); }
    EntityConfigurator& setUnitOfMeasurement(const char* unit) { return setAttribute("unit_of_measurement", unit); }
    EntityConfigurator& setAttribute(const char* key, const char* value) {
        _entity.attributes[key] = value;
        return *this;
    }
private:
    Entity& _entity;
};

class DeviceConfigurator {
public:
    DeviceConfigurator(Device& device) : _device(device) {}
    DeviceConfigurator& setName(const char* name) { _device.name = name; return *this; }
    DeviceConfigurator& setManufacturer(const char* man) { return setAttribute("manufacturer", man); }
    DeviceConfigurator& setModel(const char* model) { return setAttribute("model", model); }
    DeviceConfigurator& setSoftwareVersion(const char* sw) { return setAttribute("sw_version", sw); }
    DeviceConfigurator& setAttribute(const char* key, const char* value) {
        _device.attributes[key] = value;
        return *this;
    }
    operator Device&() { return _device; }
private:
    Device& _device;
};

// --- A Fő Osztály (a "Karmester") ---

class ESPNativeHA {
public:
    enum LogLevel { LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, };
    ESPNativeHA();
    ~ESPNativeHA();
    void begin(const char* device_name, const char* project_name = "ESPNativeHA", const char* project_version = "1.0.0", uint16_t port = 6053);
    void loop();
    void onConnect(std::function<void()> callback);
    void onDisconnect(std::function<void()> callback);
    void setLogLevel(LogLevel level);
    DeviceConfigurator createDevice(const char* unique_id);
    EntityConfigurator createSensor(Device& device, const char* entity_id, Agent<float>& agent);
    EntityConfigurator createSensor(const char* entity_id, Agent<float>& agent);
private:
    CommunicationHandler _comm_handler;
    DiscoveryHandler _discovery_handler;
    std::vector<Device*> _devices;
    std::vector<Entity*> _entities;
    std::function<void()> _on_connect_callback;
    std::function<void()> _on_disconnect_callback;
    LogLevel _logLevel = LOG_LEVEL_INFO;
    void _log_printf(LogLevel level, const char* format, ...);
};

#endif // ESP_NATIVE_HA_H