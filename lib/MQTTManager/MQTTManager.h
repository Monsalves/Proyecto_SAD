#pragma once
#include <PubSubClient.h>
extern PubSubClient client;
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
