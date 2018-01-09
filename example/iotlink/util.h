cJSON* jsonObj(cJSON *json, char *key);

char* jsonStr(cJSON *json, char *key);

int jsonInt(cJSON *json, char *key);

void IOTLINK_FREE_MESSAGE(IOTLINK_MESSAGE *message);

char* IOTLINK_PRINT_MESSAGE(IOTLINK_MESSAGE *message);