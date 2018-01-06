#include <cJSON.h>

cJSON* jsonObj(cJSON *json, char *key)
{
    return cJSON_GetObjectItem(json, key);
}

char* jsonStr(cJSON *json, char *key)
{
    return jsonObj(json, key)->valuestring;
}

int jsonInt(cJSON *json, char *key)
{
    return jsonObj(json, key)->valueint;
}