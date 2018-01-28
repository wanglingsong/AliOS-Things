#include <aos/aos.h>
#include <hal/soc/soc.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

static void gpioTriggerHandler(void *arg)
{
    void** args = arg;
    gpio_dev_t *gpio = args[0];
    LINK *link = args[1];
    uint32_t value = 0;
    hal_gpio_input_get(gpio, &value);
    IOTLINK_MESSAGE *message = aos_malloc(sizeof(IOTLINK_MESSAGE));
    message->source = MESSAGE_SOURCE_GPIO_TRIGGER;
    message->type = MESSAGE_TYPE_BOOLEAN;
    bool *bp = aos_malloc(sizeof(bool));
    *bp = value;
    message->payload = bp;
    link->message = message;
    aos_post_delayed_action(0, link->writeFunc, link);
}

void sourceGpioTrigger(void *arg)
{
    void** args = aos_malloc(sizeof(gpio_dev_t*) + sizeof(LINK*));
    gpio_dev_t *gpio;
    LINK* link = arg;
    gpio = aos_zalloc(sizeof(gpio_dev_t));
    int port = jsonInt(link->sourceConfig, "port");
    gpio->port = port;
    gpio->config = IRQ_MODE;
    hal_gpio_init(gpio);
    args[0] = gpio;
    args[1] = link;
    hal_gpio_enable_irq(gpio, IRQ_TRIGGER_BOTH_EDGES, gpioTriggerHandler, args);
}

void targetGpio(void *arg)
{
    // TODO
}