#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>
#include <sensors/dht11.h>

#define DHT11_MAX_RETRY 10

void sourceDHT11(void *arg)
{
    LINK *link = arg;
    if (!link->running)
    {
        return;
    }

    DHT11 dht;
    uint8_t port = jsonInt(link->sourceConfig, "port");

    int retry = 0;
    while (retry < DHT11_MAX_RETRY)
    {
        int32_t ret = dht11_read(&dht, port);
        if (ret == 0)
        {
            cJSON *payload = cJSON_CreateObject();
            cJSON_AddNumberToObject(payload, "temperature", dht.temperature);
            cJSON_AddNumberToObject(payload, "humidity", dht.humidity);
            link->message.source = MESSAGE_SOURCE_DHT11;
            link->message.type = MESSAGE_TYPE_JSON;
            link->message.payload = payload;
            aos_schedule_call(link->writeFunc, arg);
            break;
        }
        else
        {
            LOG("Failed to read dht11 value with error code: %d and retry: %d", ret, retry);
            retry++;
        }
    }

    int32_t interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0)
    {
        aos_post_delayed_action(interval, sourceDHT11, arg);
    }
}
