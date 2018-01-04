#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <factory.h>
#include <string.h>

static cJSON* jsonObj(cJSON *json, char *key)
{
    return cJSON_GetObjectItem(json, key);
}

static char* jsonStr(cJSON *json, char *key)
{
    return jsonObj(json, key)->valuestring;
}

static int jsonInt(cJSON *json, char *key)
{
    return jsonObj(json, key)->valueint;
}

static void readFromDummySource(void *link)
{
    LINK* linkp = link;
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("dummy"));
    linkp->payload = payload;
    aos_post_delayed_action(0, linkp->writeFunc, link);
    int interval = jsonInt(linkp->sourceConfig, "interval");
    if (interval > 0) {
        aos_post_delayed_action(interval, readFromDummySource, link);
    }
}

static void writeToDummyTarget(void *link)
{
    LINK* linkp = link;
    char *str = cJSON_Print(linkp->payload);
    LOG("Dummy Target received: %s\r\n", str);
    aos_free(str);
    cJSON_Delete(linkp->payload);
    linkp->payload = NULL;
}

static void generateTriggerPayload(bool b, LINK *link)
{
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("gpio"));
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

static void readFromGpioTrigger(void *arg)
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

static void readFromButtonSource(void *link)
{
    // TODO
}

static void writeToLedTarget(void *link)
{
    // TODO
}

static void readFromMqttSource(void *link)
{
    // TODO
}

static void writeToMqttTarget(void *link)
{
    // TODO
}

static void createSource(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->readFunc = readFromDummySource;
    } else if (strcmp(type, "gpio-trigger") == 0) {
        link->readFunc = readFromGpioTrigger;
    } else {
        // TODO
    }
}

static void createTarget(LINK *link, cJSON *config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->writeFunc = writeToDummyTarget;
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
    return link;
}