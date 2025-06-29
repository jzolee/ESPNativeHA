// =========================================================================
// File: src/drivers/MqttDriver.cpp (FRISSÍTETT a robusztusabb logikához)
// =========================================================================
#include "MqttDriver.h"
#include <ArduinoJson.h>
#include "../HAManager.h"
#include "../core/Logger.h"
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

MqttDriver::MqttDriver(const String& server, int port)
    : _server(server), _port(port), _baseTopic("homeassistant") {
    _mqttClient.onConnect(std::bind(&MqttDriver::onMqttConnect, this, std::placeholders::_1));
    _mqttClient.onDisconnect(std::bind(&MqttDriver::onMqttDisconnect, this, std::placeholders::_1));
    _mqttClient.onMessage(std::bind(&MqttDriver::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
}
void MqttDriver::setUser(const char* user, const char* password) { _mqttClient.setCredentials(user, password); }
void MqttDriver::setTopicTemplates(const MqttTopicTemplates& templates) { _topic_templates = templates; }
void MqttDriver::begin(HAManager* manager) {
    _manager = manager;
    IPAddress brokerIP;
    if (WiFi.hostByName(_server.c_str(), brokerIP)) {
        ha_log->info("MQTT Driver: Host '%s' resolved to %s", _server.c_str(), brokerIP.toString().c_str());
        _mqttClient.setServer(brokerIP, _port);
    } else {
        ha_log->warn("MQTT Driver: Could not resolve host '%s'.", _server.c_str());
        _mqttClient.setServer(_server.c_str(), _port);
        if (_manager) _manager->fireErrorEvent("Could not resolve MQTT host");
    }
    #ifdef ESP32
        uint64_t chipid = ESP.getEfuseMac();
        _chipIdHex += String((uint32_t)(chipid >> 32), HEX);
        _chipIdHex += String((uint32_t)chipid, HEX);
    #else
        _chipIdHex = String(ESP.getChipId(), HEX);
    #endif
    String clientId = "ha-native-device-" + _chipIdHex;
    _mqttClient.setClientId(clientId.c_str());
    _availabilityTopic = _topic_templates.availability_topic;
    _availabilityTopic.replace("{chip_id}", _chipIdHex);
    _mqttClient.setWill(_availabilityTopic.c_str(), 1, true, "offline");
    ha_log->info("MQTT Driver: LWT configured on topic '%s'", _availabilityTopic.c_str());
    ha_log->info("MQTT Driver: Connecting to broker...");
    _mqttClient.connect();
}
void MqttDriver::loop() {
    if (!WiFi.isConnected()) {
        return;
    }

    if (!_mqttClient.connected()) {
        if (millis() - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = millis();
            ha_log->info("MQTT Driver: Attempting to connect...");
            _mqttClient.connect();
        }
    } else {
        // JAVÍTÁS: Periodikus "életjel" (heartbeat) küldése
        if (millis() - _lastAvailabilityPublish > AVAILABILITY_PUBLISH_INTERVAL_MS) {
            _lastAvailabilityPublish = millis();
            ha_log->debug("MQTT: Publishing heartbeat availability message.");
            _mqttClient.publish(_availabilityTopic.c_str(), 1, true, "online");
        }
    }
}
void MqttDriver::onMqttConnect(bool sessionPresent) {
    ha_log->info("MQTT Driver: Connected to broker.");
    _mqttClient.publish(_availabilityTopic.c_str(), 1, true, "online");
    _lastAvailabilityPublish = millis(); // Az időzítőt most indítjuk újra

    for (const auto& pair : _command_topic_map) {
        _mqttClient.subscribe(pair.first.c_str(), 1);
        ha_log->info("MQTT: Subscribed to command topic '%s'", pair.first.c_str());
    }
    publishAllDiscoveries();
    if (_manager) _manager->fireConnectEvent();
}
void MqttDriver::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    const char* reasonStr = getDisconnectReason(reason);
    ha_log->warn("MQTT Driver: Disconnected. Reason: %s", reasonStr);
    for(auto* dev : _manager->getDevices()) {
        for(auto* ent : dev->getEntities()) ent->setDiscoveryPublished(false);
    }
    for(auto* ent : _manager->getStandaloneEntities()) ent->setDiscoveryPublished(false);
    _lastReconnectAttempt = millis();
    if (_manager) _manager->fireDisconnectEvent();
}
void MqttDriver::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    String payloadStr;
    payloadStr.reserve(len + 1);
    for(size_t i = 0; i < len; i++) {
        payloadStr += (char)payload[i];
    }
    ha_log->debug("MQTT: Message received on topic '%s': %s", topic, payloadStr.c_str());
    auto it = _command_topic_map.find(topic);
    if (it != _command_topic_map.end()) {
        it->second->onCommand(payloadStr);
    } else {
        ha_log->warn("MQTT: Received message on an unknown command topic: %s", topic);
    }
}
void MqttDriver::registerEntity(BaseEntity* entity) {
    entity->attachToAgent([this, entity]() {
        this->publishState(entity);
    });
    if (entity->isCommandable()) {
        String cmd_topic = getCommandTopic(entity);
        _command_topic_map[cmd_topic] = entity;
        if (_mqttClient.connected()) {
            _mqttClient.subscribe(cmd_topic.c_str(), 1);
            ha_log->info("MQTT: Subscribed to command topic '%s'", cmd_topic.c_str());
        }
    }
}
void MqttDriver::publishAllDiscoveries() {
    if(!_mqttClient.connected()) return;
    ha_log->info("MQTT: Publishing all discoveries...");
    for(Device* device : _manager->getDevices()) {
        for(BaseEntity* entity : device->getEntities()) {
            publishStandardDiscovery(entity);
        }
    }
    for(BaseEntity* entity : _manager->getStandaloneEntities()) {
        publishStandardDiscovery(entity);
    }
}
void MqttDriver::publishStandardDiscovery(BaseEntity* entity) {
    if (entity->isDiscoveryPublished()) return;
    String topic = getStandardDiscoveryTopic(entity);
    JsonDocument doc;
    doc["name"] = entity->getName();
    doc["uniq_id"] = entity->getUniqueId();
    doc["avty_t"] = _availabilityTopic;
    if (entity->usesJsonAttributes()) {
        doc["schema"] = "json";
        doc["json_attr_t"] = getStateTopic(entity);
    } else {
        doc["stat_t"] = getStateTopic(entity);
    }
    if (entity->isCommandable()) {
        doc["cmd_t"] = getCommandTopic(entity);
    }
    for (const auto& attr : entity->getAttributes()) {
        if (attr.second.startsWith("[")) {
            JsonDocument temp_doc;
            deserializeJson(temp_doc, attr.second);
            if (!temp_doc.isNull()) {
                doc[attr.first] = temp_doc.as<JsonArray>();
            }
        } else {
            doc[attr.first] = attr.second;
        }
    }
    if (entity->getDevice()) {
        JsonObject device_doc = doc["dev"].to<JsonObject>();
        entity->getDevice()->serializeToJson(device_doc);
    }
    String payload;
    serializeJson(doc, payload);
    if(doc.overflowed()){
      ha_log->error("MQTT: Discovery JSON for '%s' overflowed!", entity->getUniqueId().c_str());
      return;
    }
    _mqttClient.publish(topic.c_str(), 1, true, payload.c_str());
    entity->setDiscoveryPublished(true);
    ha_log->info("MQTT: Standard discovery published for '%s'", entity->getUniqueId().c_str());
    publishState(entity);
}
void MqttDriver::publishState(BaseEntity* entity) {
    if (!_mqttClient.connected()) return;
    String topic = getStateTopic(entity);
    String state = entity->getStateAsString();
    _mqttClient.publish(topic.c_str(), 1, true, state.c_str());
}
String MqttDriver::getStandardDiscoveryTopic(BaseEntity* entity) {
    return _baseTopic + "/" + entity->getComponentType() + "/" + entity->getUniqueId() + "/config";
}
String MqttDriver::getStateTopic(BaseEntity* entity) {
    String topic = _topic_templates.state_topic;
    topic.replace("{unique_id}", entity->getUniqueId());
    return topic;
}
String MqttDriver::getCommandTopic(BaseEntity* entity) {
    String topic = _topic_templates.command_topic;
    topic.replace("{unique_id}", entity->getUniqueId());
    return topic;
}
const char* MqttDriver::getDisconnectReason(AsyncMqttClientDisconnectReason reason) {
  switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED: return "TCP Disconnected";
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION: return "Unacceptable Protocol Version";
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED: return "Identifier Rejected";
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE: return "Server Unavailable";
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS: return "Malformed Credentials";
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED: return "Not Authorized";
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE: return "ESP8266 Not Enough Space";
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT: return "TLS Bad Fingerprint";
    default: return "Unknown";
  }
}