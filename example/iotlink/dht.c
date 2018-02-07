#include <stdio.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>
#include <drivers/dht11.h>

void sourceDHT11(void *arg)
{
    LINK *link = arg;
    if (!link->running)
    {
        return;
    }

    gpio_dev_t *gpio;
    gpio = aos_malloc(sizeof(gpio_dev_t));
    int port = jsonInt(link->sourceConfig, "port");
    gpio->port = port;

    DHT11 dht;
    if (dht11_read(&dht, gpio))
    {
        int strSize = sizeof(char) * 64;
        char *msgStr = aos_zalloc(strSize);
        // TODO in JSON format
        snprintf(msgStr, strSize, "{\"temperature\":%d,\"humidity\":%d}", dht.temperature, dht.humidity);

        IOTLINK_MESSAGE *message = aos_malloc(sizeof(IOTLINK_MESSAGE));
        message->source = MESSAGE_SOURCE_DHT11;
        message->type = MESSAGE_TYPE_STRING;
        message->payload = msgStr;
        link->message = message;
        aos_post_delayed_action(0, link->writeFunc, arg);
    }
    else
    {
        LOG("Failed to read dht11 value");
    }

    int interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0)
    {
        aos_post_delayed_action(interval, sourceDHT11, arg);
    }
}
