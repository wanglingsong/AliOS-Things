#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

void sourceDummy(void *arg)
{
    LINK *link = arg;
    if (!link->running)
    {
        return;
    }
    link->message.source = MESSAGE_SOURCE_DUMMY;
    link->message.type = MESSAGE_TYPE_STRING;
    int len = sizeof(char) * (18 + 1);
    link->message.payload = aos_malloc(len);
    memcpy(link->message.payload, "It's dummy message", len);
    aos_schedule_call(link->writeFunc, arg);
    int interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0)
    {
        aos_post_delayed_action(interval, sourceDummy, arg);
    }
}

void targetDummy(void *arg)
{
    // LOG("Dummy Target start");
    LINK *link = arg;
    char *str = IOTLINK_PRINT_MESSAGE(&(link->message));
    LOG("Dummy Target received message:%s", str);
    aos_free(str);
    IOTLINK_FREE_MESSAGE(&(link->message));
    // link->message = NULL;
    // LOG("Dummy Target end");
}