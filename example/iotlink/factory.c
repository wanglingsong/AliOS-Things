#include <string.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>
#include <dummy.h>
#include <gpio.h>
#include <dht.h>

#if defined(IOT_LINK_MQTT)
#include <mqtt.h>
#endif

#if defined(IOT_LINK_MQTT)

static void setMqttSource(input_event_t *event, void *arg)
{
    LINK *link = arg;
    link->readFunc = sourceMqtt;
    LOG("set mqtt readFunc");
    aos_post_event(EV_LINK_UPDATED, 0, 0);
}

static void setMqttTarget(input_event_t *event, void *arg)
{
    LINK *link = arg;
    link->writeFunc = targetMqtt;
    LOG("set mqtt writeFunc");
    aos_post_event(EV_LINK_UPDATED, 0, 0);
}

#endif

static void createSource(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0)
    {
        link->readFunc = sourceDummy;
        LOG("Post dummy source set");
        aos_post_event(EV_LINK_UPDATED, 0, 0);
    }
    else if (strcmp(type, "irq") == 0)
    {
        link->readFunc = sourceGpioTrigger;
        LOG("Post irq source set");
        aos_post_event(EV_LINK_UPDATED, 0, 0);
    }
    else if (strcmp(type, "dht11") == 0)
    {
        link->readFunc = sourceDHT11;
        LOG("Post dht11 source set");
        aos_post_event(EV_LINK_UPDATED, 0, 0);

#if defined(IOT_LINK_MQTT)
    }
    else if (strcmp(type, "mqtt") == 0)
    {
        aos_register_event_filter(EV_MQTT_CONNETED, setMqttSource, link);
#endif
    }
    else
    {
        // TODO
    }
}

static void createTarget(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0)
    {
        link->writeFunc = targetDummy;
        LOG("Post dummy target set");
        aos_post_event(EV_LINK_UPDATED, 0, 0);
    }
    else if (strcmp(type, "gpio") == 0)
    {
        link->writeFunc = targetGpio;
        LOG("Post gpio target set");
        aos_post_event(EV_LINK_UPDATED, 0, 0);

#if defined(IOT_LINK_MQTT)
    }
    else if (strcmp(type, "mqtt") == 0)
    {
        LOG("Creating mqtt target");
        aos_register_event_filter(EV_MQTT_CONNETED, setMqttTarget, link);
#endif
    }
    else
    {
        // TODO
    }
}

LINK *createLink(cJSON *config)
{
    LINK *link = aos_zalloc(sizeof(LINK));
    link->running = false;
    link->sourceConfig = jsonObj(config, "source");
    link->targetConfig = jsonObj(config, "target");
    return link;
}

void setupLink(LINK *link)
{
    createSource(link, link->sourceConfig);
    createTarget(link, link->targetConfig);
}

TRANSPORT *createTransports(cJSON *config)
{
    if (config == NULL)
    {
        return NULL;
    }
    char *type = jsonStr(config, "type");

#if defined(IOT_LINK_MQTT)
    if (strcmp(type, "mqtt") == 0)
    {
        return createMqttTransport(config);
    }
#endif

    return NULL;
}