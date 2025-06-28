// =========================================================================
// File: src/entities/Device.cpp
// Leírás: A Device osztály implementációja.
// =========================================================================
#include "Device.h"
#include "BaseEntity.h" // A BaseEntity teljes definíciójához

Device::Device(const String& unique_id) : _unique_id(unique_id) {}

void Device::addEntity(BaseEntity* entity) {
    _entities.push_back(entity);
}

void Device::serializeToJson(JsonObject& doc) const {
    doc["ids"] = _unique_id;
    doc["name"] = _name;
    for (const auto& attr : _attributes) {
        doc[attr.first] = attr.second;
    }
}