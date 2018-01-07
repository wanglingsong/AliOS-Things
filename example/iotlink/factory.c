#include <string.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

#if defined(IOT_LINK_MQTT)
#include <mqtt.h>
#endif

static void readFromDummySource(void *arg)
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
        aos_post_delayed_action(interval, readFromDummySource, arg);
    }
}

static void writeToDummyTarget(void *arg)
{
    LINK* link = arg;
    char *str = cJSON_Print(link->payload);
    LOG("Dummy Target received: %s\r\n", str);
    aos_free(str);
    cJSON_Delete(link->payload);
    link->payload = NULL;
}

static void generateTriggerPayload(bool b, LINK *link)
{
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("button"));
    cJSON_AddItemToObject(payload, "type", cJSON_CreateString("boolean"));
    cJSON_AddItemToObject(payload, "payload", b ? cJSON_CreateTrue() : cJSON_CreateFalse());
    link->payload = payload;
    aos_post_delayed_action(0, link->writeFunc, link);
}

static void irg_rising_handler(void *arg)
{
    generateTriggerPayload(true, arg);
}

static void irg_falling_handler(void *arg)
{
    generateTriggerPayload(false, arg);
}

static void readFromButtonSource(void *arg)
{
    LINK* linkp = arg;
    gpio_dev_t *gpio = aos_malloc(sizeof(gpio_dev_t));
    int port = jsonInt(linkp->sourceConfig, "port");
    gpio->port = port;
    gpio->config = INPUT_PULL_UP;
    hal_gpio_init(gpio);
    hal_gpio_enable_irq(gpio, IRQ_TRIGGER_RISING_EDGE, irg_rising_handler, link);
    hal_gpio_enable_irq(gpio, IRQ_TRIGGER_FALLING_EDGE, irg_falling_handler, link);
}

static void writeToLedTarget(void *link)
{
    // TODO
}

#if defined(IOT_LINK_MQTT)

static void setMqttSource(input_event_t *event, void *arg)
{
    LINK* link = arg;
    link->readFunc = readFromMqttSource;
    aos_post_event(EV_LINK_UPDATED, 0, 0);
}

static void setMqttTarget(input_event_t *event, void *arg)
{
    LINK* link = arg;
    link->writeFunc = writeToMqttTarget;
    aos_post_event(EV_LINK_UPDATED, 0, 0);
}

#endif

static void createSource(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->readFunc = readFromDummySource;
        aos_post_event(EV_LINK_UPDATED, 0, 0);
    // } else if (strcmp(type, "button") == 0) {
    //     link->readFunc = readFromButtonSource;

#if defined(IOT_LINK_MQTT)
    } else if (strcmp(type, "mqtt") == 0) {
        aos_register_event_filter(EV_MQTT_CONNETED, setMqttSource, link);
#endif

    } else {
        // TODO
    }
}

static void createTarget(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->writeFunc = writeToDummyTarget;
        aos_post_event(EV_LINK_UPDATED, 0, 0);
        
#if defined(IOT_LINK_MQTT)
    } else if (strcmp(type, "mqtt") == 0) {
        aos_register_event_filter(EV_MQTT_CONNETED, setMqttTarget, link);
#endif

    } else {
        // TODO
    }
}

LINK* iotlink_createLink(cJSON *config)
{
    LINK *link = aos_malloc(sizeof(LINK));
    cJSON *sourceConfig = jsonObj(config, "source");
    link->sourceConfig = sourceConfig;
    cJSON *targetConfig = jsonObj(config, "target");
    link->targetConfig = targetConfig;
    createSource(link, sourceConfig);
    createTarget(link, targetConfig);
    link->running = false;
    return link;
}

TRANSPORT* iotlink_createTransports(cJSON *config)
{
    char *type = jsonStr(config, "type");

#if defined(IOT_LINK_MQTT)
    if (strcmp(type, "mqtt") == 0) {
        return createMqttTransport(config);
    }
#endif

    return NULL;    
}