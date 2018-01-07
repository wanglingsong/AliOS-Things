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
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("dummy"));
    link->payload = payload;
    aos_post_delayed_action(0, link->writeFunc, arg);
    int interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0) {
        aos_post_delayed_action(interval, sourceDummy, arg);
    }
}

void targetDummy(void *arg)
{
    LINK* link = arg;
    char *str = cJSON_Print(link->payload);
    LOG("Dummy Target received: %s\r\n", str);
    aos_free(str);
    cJSON_Delete(link->payload);
    link->payload = NULL;
}