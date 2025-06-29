// =========================================================================
// File: src/drivers/BaseDriver.h
// =========================================================================
#pragma once

#include "../entities/BaseEntity.h"

class HAManager;

class BaseDriver {
public:
    virtual ~BaseDriver() {}
    virtual void begin(HAManager* manager) = 0;
    virtual void loop() = 0;
    virtual void registerEntity(BaseEntity* entity) = 0;
};