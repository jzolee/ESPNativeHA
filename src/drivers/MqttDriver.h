// =========================================================================
// File: src/drivers/MqttDriver.h (FRISSÍTETT a robusztusabb logikához)
// =========================================================================
#pragma once

#include <AsyncMqttClient.h>
#include "BaseDriver.h"
#include <vector>
#include <map>

struct MqttTopicTemplates {
    String state_topic = "espnativeha/{unique_id}/state";
    String command_topic = "espnativeha/{unique_id}/command";
    String availability_topic = "espnativeha/{chip_id}/status";
};

class MqttDriver : public BaseDriver {
private:
    AsyncMqttClient _mqttClient;
    String _server;
    int _port;
    String _baseTopic;
    HAManager* _manager = nullptr;
    
    uint32_t _lastReconnectAttempt = 0;
    
    // JAVÍTÁS: Új heartbeat időzítő
    uint32_t _lastAvailabilityPublish = 0;
    const uint32_t AVAILABILITY_PUBLISH_INTERVAL_MS = 60000; // 1 perc

    String _availabilityTopic;
    String _chipIdHex;

    MqttTopicTemplates _topic_templates;

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
    void setTopicTemplates(const MqttTopicTemplates& templates);
    void begin(HAManager* manager) override;
    void loop() override;
    void registerEntity(BaseEntity* entity) override;
};