// =========================================================================
// File: src/drivers/MqttDriver.cpp
// Leírás: Az MqttDriver implementációja, parancskezeléssel
// =========================================================================
#include "MqttDriver.h"
#include <ArduinoJson.h>
#include "../HAManager.h"
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

void MqttDriver::setUser(const char* user, const char* password) {
    _mqttClient.setCredentials(user, password);
}

void MqttDriver::begin(HAManager* manager) {
    _manager = manager;
    
    IPAddress brokerIP;
    if (WiFi.hostByName(_server.c_str(), brokerIP)) {
        Serial.printf("MQTT Driver: Host '%s' resolved to %s\n", _server.c_str(), brokerIP.toString().c_str());
        _mqttClient.setServer(brokerIP, _port);
    } else {
        Serial.printf("MQTT Driver: WARNING - Could not resolve host '%s'. Trying with hostname directly.\n", _server.c_str());
        _mqttClient.setServer(_server.c_str(), _port);
    }

    String clientId = "ha-native-device-";
    String chipIdHex;
    #ifdef ESP32
        uint64_t chipid = ESP.getEfuseMac();
        chipIdHex += String((uint32_t)(chipid >> 32), HEX);
        chipIdHex += String((uint32_t)chipid, HEX);
    #else // ESP8266
        chipIdHex = String(ESP.getChipId(), HEX);
    #endif
    clientId += chipIdHex;

    _mqttClient.setClientId(clientId.c_str());
    
    _availabilityTopic = "espnativeha/" + chipIdHex + "/status";
    _mqttClient.setWill(_availabilityTopic.c_str(), 1, true, "offline");
    Serial.printf("MQTT Driver: LWT configured on topic '%s'\n", _availabilityTopic.c_str());

    Serial.println("MQTT Driver: Connecting to broker...");
    _mqttClient.connect();
}

void MqttDriver::loop() {
    if (_reconnecting && (millis() - _lastReconnectAttempt > 5000)) {
        _lastReconnectAttempt = millis();
        if (WiFi.isConnected()) {
            Serial.println("MQTT Driver: Attempting to reconnect...");
            _mqttClient.connect();
        } else {
            Serial.println("MQTT Driver: Cannot reconnect, WiFi is not connected.");
        }
    }
}

void MqttDriver::onMqttConnect(bool sessionPresent) {
    Serial.println("MQTT Driver: Connected to broker.");
    _reconnecting = false;
    
    _mqttClient.publish(_availabilityTopic.c_str(), 1, true, "online");
    
    for (auto const& [topic, entity] : _command_topic_map) {
        _mqttClient.subscribe(topic.c_str(), 1);
        Serial.printf("MQTT: Subscribed to command topic '%s'\n", topic.c_str());
    }
    
    publishAllDiscoveries();
}

void MqttDriver::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("MQTT Driver: Disconnected. Reason: %s\n", getDisconnectReason(reason));
    _reconnecting = true;

    for(auto* dev : _manager->getDevices()) {
        for(auto* ent : dev->getEntities()) ent->setDiscoveryPublished(false);
    }
    for(auto* ent : _manager->getStandaloneEntities()) ent->setDiscoveryPublished(false);
    
    _lastReconnectAttempt = millis();
}

void MqttDriver::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    String payloadStr;
    payloadStr.reserve(len + 1);
    for(size_t i = 0; i < len; i++) {
        payloadStr += (char)payload[i];
    }

    Serial.printf("MQTT: Message received on topic '%s': %s\n", topic, payloadStr.c_str());

    auto it = _command_topic_map.find(topic);
    if (it != _command_topic_map.end()) {
        it->second->onCommand(payloadStr);
    } else {
        Serial.println("MQTT: Received message on an unknown command topic.");
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
            Serial.printf("MQTT: Subscribed to command topic '%s'\n", cmd_topic.c_str());
        }
    }
}

void MqttDriver::publishAllDiscoveries() {
    if(!_mqttClient.connected()) return;

    Serial.println("MQTT: Publishing all discoveries...");
    
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
    doc["stat_t"] = getStateTopic(entity);
    doc["avty_t"] = _availabilityTopic;
    
    if (entity->isCommandable()) {
        doc["cmd_t"] = getCommandTopic(entity);
    }

    for (const auto& attr : entity->getAttributes()) {
        doc[attr.first] = attr.second;
    }
    
    if (entity->getDevice()) {
        JsonObject device_doc = doc["dev"].to<JsonObject>();
        entity->getDevice()->serializeToJson(device_doc);
    }
    
    String payload;
    serializeJson(doc, payload);

    if(doc.overflowed()){
      Serial.printf("MQTT ERROR: Discovery JSON for '%s' overflowed!\n", entity->getUniqueId().c_str());
      return;
    }

    _mqttClient.publish(topic.c_str(), 1, true, payload.c_str());
    entity->setDiscoveryPublished(true);
    Serial.printf("MQTT: Standard discovery published for '%s'\n", entity->getUniqueId().c_str());
    
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
    return "espnativeha/" + entity->getUniqueId() + "/state";
}

String MqttDriver::getCommandTopic(BaseEntity* entity) {
    return "espnativeha/" + entity->getUniqueId() + "/command";
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