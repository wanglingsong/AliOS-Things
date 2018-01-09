#include <stdio.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>

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

void IOTLINK_FREE_MESSAGE(IOTLINK_MESSAGE *message)
{
    // LOG("Try to free message");
    if (message->type == MESSAGE_TYPE_JSON) {
        cJSON_Delete(message->payload);
    } else {
        aos_free(message->payload);
    }
    aos_free(message);
    // LOG("Free message success");
}

char* IOTLINK_PRINT_MESSAGE(IOTLINK_MESSAGE *message)
{
    int strSize = sizeof(char) * 128;
    char *msgStr = aos_zalloc(strSize);
    if (message->type == MESSAGE_TYPE_BOOLEAN) {
        snprintf(msgStr, strSize, "{\"source\":%d,\"type\":%d,\"payload\":%d}", message->source, message->type, message->payload);
    } else if (message->type == MESSAGE_TYPE_NUMBER) {
        snprintf(msgStr, strSize, "{\"source\":%d,\"type\":%d,\"payload\":%d}", message->source, message->type, message->payload);
    } else if (message->type == MESSAGE_TYPE_STRING) {
        snprintf(msgStr, strSize, "{\"source\":%d,\"type\":%d,\"payload\":\"%s\"}", message->source, message->type, message->payload);
    } else if (message->type == MESSAGE_TYPE_JSON) {
        char* jsonTmp = cJSON_PrintUnformatted(message->payload);
        snprintf(msgStr, strSize, "{\"source\":%d,\"type\":%d,\"payload\":%s}", message->source, message->type, jsonTmp);
        aos_free(jsonTmp);
    }
    // int msg_len = snprintf(msgStr, sizeof(msgStr), "{\"source\":%d,\"type\":%d,\"payload\":%s}", message->source, message->type, payload);
    // if (msg_len < 0) {
    //     LOG("Failed to print message");
    // }
    return msgStr;
}