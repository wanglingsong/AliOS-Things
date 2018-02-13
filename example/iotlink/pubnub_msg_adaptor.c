#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

char *encode_iotlink_message(IOTLINK_MESSAGE *message)
{
    return IOTLINK_PRINT_MESSAGE(message);
}

int8_t decode_iotlink_message(IOTLINK_MESSAGE *message, const char *str)
{
    IOTLINK_FREE_MESSAGE(message);
    cJSON *json = cJSON_Parse(str);
    message->source = MESSAGE_SOURCE_MQTT;
    message->type = jsonInt(json, "type");
    if (message->type == MESSAGE_TYPE_BOOLEAN)
    {
        bool *bp = aos_malloc(sizeof(bool));
        *bp = jsonInt(json, "payload");
        message->payload = (void *)bp;
    }
    if (message->type == MESSAGE_TYPE_JSON)
    {
        cJSON *duplicate = cJSON_Duplicate(jsonObj(json, "payload"), true);
        message->payload = (void *)duplicate;
    }
    else
    {
        LOG("Invalid type");
        return -1;
    }
    cJSON_Delete(json);
    return 0;
}