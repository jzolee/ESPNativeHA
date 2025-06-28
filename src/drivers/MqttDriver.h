// =========================================================================
// File: src/drivers/MqttDriver.h
// Leírás: MQTT Driver, parancskezeléssel kiegészítve
// =========================================================================
#pragma once

#include <AsyncMqttClient.h>
#include "BaseDriver.h"
#include <vector>
#include <map>

class MqttDriver : public BaseDriver {
private:
    AsyncMqttClient _mqttClient;
    String _server;
    int _port;
    String _baseTopic;
    HAManager* _manager = nullptr;
    
    bool _reconnecting = false;
    uint32_t _lastReconnectAttempt = 0;
    
    String _availabilityTopic;

    std::map<String, BaseEntity*> _command_topic_map;

    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

    void publishAllDiscoveries();
    void publishStandardDiscovery(BaseEntity* entity);

    void publishState(BaseEntity* entity);

    String getStandardDiscoveryTopic(BaseEntity* entity);
    String getStateTopic(BaseEntity* entity);
    String getCommandTopic(BaseEntity* entity);
    
    const char* getDisconnectReason(AsyncMqttClientDisconnectReason reason);

public:
    MqttDriver(const String& server, int port = 1883);
    
    void setUser(const char* user, const char* password);
    void begin(HAManager* manager) override;
    void loop() override;
    void registerEntity(BaseEntity* entity) override;
};