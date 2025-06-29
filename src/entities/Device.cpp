// =========================================================================
// File: src/entities/Device.cpp
// =========================================================================
#include "Device.h"
#include "BaseEntity.h"

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