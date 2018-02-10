void sourceMqtt(void *link);

void targetMqtt(void *link);

TRANSPORT *createMqttTransport(cJSON *config);