// #include <types.h>

void readFromMqttSource(void *link);

void writeToMqttTarget(void *link);

TRANSPORT* createMqttTransport(cJSON *config);