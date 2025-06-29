// =========================================================================
// File: src/entities/ComplexEntities.h
// =========================================================================
#pragma once

#include "BaseEntity.h"
#include <ArduinoJson.h>

class Light : public BaseEntity {
private:
    Agent<bool>& _stateAgent;
    Agent<int>& _brightnessAgent;
    Agent<String> _internalJsonAgent; 

public:
    Light(const String& unique_id, Agent<bool>& stateAgent, Agent<int>& brightnessAgent)
        : _stateAgent(stateAgent), _brightnessAgent(brightnessAgent), _internalJsonAgent("") {
        this->_unique_id = unique_id;
        
        this->_attributes["schema"] = "json";
        this->_attributes["supported_color_modes"] = "[\"brightness\"]";
        
        auto update_cb = [this](){ this->updateJsonState(); };
        _stateAgent.attach([update_cb](bool){ update_cb(); });
        _brightnessAgent.attach([update_cb](int){ update_cb(); });
        
        updateJsonState();
    }

    const char* getComponentType() const override { return "light"; }
    bool isCommandable() const override { return true; }
    bool usesJsonAttributes() const override { return true; }
    
    String getStateAsString() const override { return _internalJsonAgent.get(); }

    void attachToAgent(std::function<void()> on_change) override {
        _internalJsonAgent.attach([on_change](String){ on_change(); });
    }

    void onCommand(const String& payload) override {
        JsonDocument doc;
        deserializeJson(doc, payload);

        if (!doc["state"].isNull()) {
            _stateAgent.set(doc["state"].as<String>() == "ON");
        }
        if (!doc["brightness"].isNull()) {
            _brightnessAgent.set(doc["brightness"].as<int>());
        }
    }
    
    void updateJsonState() {
        JsonDocument doc;
        doc["state"] = _stateAgent.get() ? "ON" : "OFF";
        doc["brightness"] = _brightnessAgent.get();
        String jsonState;
        serializeJson(doc, jsonState);
        _internalJsonAgent.set(jsonState);
    }
};