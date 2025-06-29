// =========================================================================
// File: src/entities/SpecializedEntities.h
// =========================================================================
#pragma once

#include "TypedEntity.h"

template<typename T>
class Sensor : public TypedEntity<T> {
public:
    Sensor(const String& unique_id, Agent<T>& agent) : TypedEntity<T>(unique_id, "sensor", agent) {}
};

class BinarySensor : public TypedEntity<bool> {
public:
    BinarySensor(const String& unique_id, Agent<bool>& agent) : TypedEntity<bool>(unique_id, "binary_sensor", agent) {}
};

class Switch : public TypedEntity<bool> {
public:
    Switch(const String& unique_id, Agent<bool>& agent) : TypedEntity<bool>(unique_id, "switch", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override {
        if (payload.equalsIgnoreCase("ON")) this->_agent.set(true);
        else if (payload.equalsIgnoreCase("OFF")) this->_agent.set(false);
    }
};

template<typename T>
class Number : public TypedEntity<T> {
public:
    Number(const String& unique_id, Agent<T>& agent) : TypedEntity<T>(unique_id, "number", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override {
        if (std::is_same<T, int>::value || std::is_same<T, long>::value) {
            this->_agent.set(payload.toInt());
        } else if (std::is_same<T, float>::value || std::is_same<T, double>::value) {
            this->_agent.set(payload.toFloat());
        }
    }
};

class Select : public TypedEntity<String> {
public:
    Select(const String& unique_id, Agent<String>& agent) : TypedEntity<String>(unique_id, "select", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override { this->_agent.set(payload); }
};

class Button : public TypedEntity<bool> {
public:
    Button(const String& unique_id, Agent<bool>& agent) : TypedEntity<bool>(unique_id, "button", agent) {}
    bool isCommandable() const override { return true; }
    String getStateAsString() const override { return ""; }
    void onCommand(const String& payload) override {
        this->_agent.set(!this->_agent.get());
    }
};

class Text : public TypedEntity<String> {
public:
    Text(const String& unique_id, Agent<String>& agent) : TypedEntity<String>(unique_id, "text", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override { this->_agent.set(payload); }
};

class Lock : public TypedEntity<bool> {
public:
    Lock(const String& unique_id, Agent<bool>& agent) : TypedEntity<bool>(unique_id, "lock", agent) {}
    bool isCommandable() const override { return true; }
    String getStateAsString() const override {
        return this->_agent.get() ? "LOCKED" : "UNLOCKED";
    }
    void onCommand(const String& payload) override {
        if (payload.equalsIgnoreCase("LOCK")) this->_agent.set(true);
        else if (payload.equalsIgnoreCase("UNLOCK")) this->_agent.set(false);
    }
};

class Cover : public TypedEntity<String> {
public:
    Cover(const String& unique_id, Agent<String>& agent) : TypedEntity<String>(unique_id, "cover", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override {
        this->_agent.set(payload);
    }
};

// JAVÍTÁS: Egyéni entitás (Siren)
class Siren : public TypedEntity<bool> {
public:
    Siren(const String& unique_id, Agent<bool>& agent) : TypedEntity<bool>(unique_id, "switch", agent) {}
    bool isCommandable() const override { return true; }
    void onCommand(const String& payload) override {
        if (payload.equalsIgnoreCase("ON")) this->_agent.set(true);
        else if (payload.equalsIgnoreCase("OFF")) this->_agent.set(false);
    }
};
