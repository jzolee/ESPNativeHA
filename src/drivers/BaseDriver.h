// =========================================================================
// File: src/drivers/BaseDriver.h
// Leírás: Absztrakt "szerződés" (interfész) a driverekhez.
// =========================================================================
#pragma once

#include "../entities/BaseEntity.h"

class HAManager;

class BaseDriver {
public:
    virtual ~BaseDriver() {}
    virtual void begin(HAManager* manager) = 0;
    virtual void loop() = 0;
    
    // Ez a függvény a "ragasztó" az entitás és a driver között
    virtual void registerEntity(BaseEntity* entity) = 0;
};