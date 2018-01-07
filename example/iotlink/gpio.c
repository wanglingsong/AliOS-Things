#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

static void gpioTriggerHandler(bool b, LINK *link)
{
    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("button"));
    cJSON_AddItemToObject(payload, "type", cJSON_CreateString("boolean"));
    cJSON_AddItemToObject(payload, "payload", b ? cJSON_CreateTrue() : cJSON_CreateFalse());
    link->payload = payload;
    aos_post_delayed_action(0, link->writeFunc, link);
}

static void gpioRisingHandler(void *arg)
{
    gpioTriggerHandler(true, arg);
}

static void gpioFallingHandler(void *arg)
{
    gpioTriggerHandler(false, arg);
}

void sourceGpioTrigger(void *arg)
{
    LINK* link = arg;
    gpio_dev_t *gpio = aos_zalloc(sizeof(gpio_dev_t));
    int port = jsonInt(link->sourceConfig, "port");
    gpio->port = port;
    gpio->config = INPUT_PULL_UP;
    hal_gpio_init(gpio);
    hal_gpio_enable_irq(gpio, IRQ_TRIGGER_RISING_EDGE, gpioRisingHandler, arg);
    hal_gpio_enable_irq(gpio, IRQ_TRIGGER_FALLING_EDGE, gpioFallingHandler, arg);
}

void targetGpio(void *arg)
{
    // TODO
}