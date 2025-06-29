// =========================================================================
// File: src/entities/TypedEntity.h
// =========================================================================
#pragma once

#include "BaseEntity.h"

template <typename T>
class TypedEntity : public BaseEntity {
protected:
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
};