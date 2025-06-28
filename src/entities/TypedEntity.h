// =========================================================================
// File: src/entities/TypedEntity.h
// Leírás: Sablonos entitás osztály (FRISSÍTETT a number típushoz)
// =========================================================================
#pragma once

#include "BaseEntity.h"

template <typename T>
class TypedEntity : public BaseEntity {
private:
    Agent<T>& _agent;
    String _component_type;

public:
    TypedEntity(const String& unique_id, const char* component_type, Agent<T>& agent) 
        : _agent(agent), _component_type(component_type) {
        this->_unique_id = unique_id;
    }

    const char* getComponentType() const override {
        return _component_type.c_str();
    }
    
    String getStateAsString() const override {
        if (std::is_same<T, bool>::value) {
            return _agent.get() ? "ON" : "OFF";
        }
        return String(_agent.get());
    }
    
    void attachToAgent(std::function<void()> on_change) override {
        _agent.attach([on_change](T /*new_value*/) {
            on_change();
        });
    }

    bool isCommandable() const override {
        // JAVÍTÁS: A number is parancsolható
        return _component_type == "switch" || _component_type == "light" || _component_type == "number";
    }
    
    void onCommand(const String& payload) override {
        if (std::is_same<T, bool>::value) {
            if (payload.equalsIgnoreCase("ON")) {
                _agent.set(true);
            } else if (payload.equalsIgnoreCase("OFF")) {
                _agent.set(false);
            }
        } 
        // JAVÍTÁS: Szám típusú parancsok feldolgozása
        else if (std::is_same<T, int>::value || std::is_same<T, long>::value) {
            _agent.set(payload.toInt());
        } else if (std::is_same<T, float>::value || std::is_same<T, double>::value) {
            _agent.set(payload.toFloat());
        }
    }
};