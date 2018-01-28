#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

void sourceDummy(void *arg)
{
    LINK* link = arg;
    if (!link->running) {
        return;
    }
    IOTLINK_MESSAGE *message = aos_malloc(sizeof(IOTLINK_MESSAGE));
    message->source = MESSAGE_SOURCE_DUMMY;
    message->type = MESSAGE_TYPE_STRING;
    int len = sizeof(char)*(18 + 1);
    message->payload = aos_malloc(len);
    memcpy(message->payload, "It's dummy message", len);
    // cJSON *payload = cJSON_CreateObject();
    // cJSON_AddItemToObject(payload, "source", cJSON_CreateString("dummy"));
    // link->payload = payload;
    link->message = message;
    aos_post_delayed_action(0, link->writeFunc, arg);
    int interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0) {
        aos_post_delayed_action(interval, sourceDummy, arg);
    }
}

void targetDummy(void *arg)
{
    LINK* link = arg;
    char *str = IOTLINK_PRINT_MESSAGE(link->message);    
    LOG("Dummy Target received message:%s", str);
    aos_free(str);
    IOTLINK_FREE_MESSAGE(link->message);
    link->message = NULL;
}