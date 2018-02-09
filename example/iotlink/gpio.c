#include <aos/aos.h>
#include <hal/soc/gpio.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

static void gpioTriggerHandler(void *arg)
{
    // LOG("Begin gpioTriggerHandler");
    LINK *link = arg;
    int port = jsonInt(link->sourceConfig, "port");
    gpio_dev_t gpio;
    gpio.port = port;
    uint32_t value;
    bool *bp = aos_malloc(sizeof(bool));

    hal_gpio_input_get(&gpio, &value);
    // LOG("Got irq value: %d", value);
    link->message.source = MESSAGE_SOURCE_GPIO_TRIGGER;
    link->message.type = MESSAGE_TYPE_BOOLEAN;
    *bp = value;
    link->message.payload = bp;
    aos_schedule_call(link->writeFunc, arg);
    // LOG("End gpioTriggerHandler");
}

void sourceGpioTrigger(void *arg)
{
    LINK *link = arg;
    int port = jsonInt(link->sourceConfig, "port");
    gpio_dev_t gpio;
    gpio.port = port;
    gpio.config = IRQ_MODE;
    hal_gpio_init(&gpio);
    hal_gpio_enable_irq(&gpio, IRQ_TRIGGER_BOTH_EDGES, gpioTriggerHandler, arg);
}

void targetGpio(void *arg)
{
    // LOG("targetGpio start");
    LINK *link = arg;
    int port = jsonInt(link->targetConfig, "port");
    gpio_dev_t gpio;
    gpio.port = port;
    gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&gpio);
    if (link->message.type == MESSAGE_TYPE_BOOLEAN)
    {
        if (*((bool *)link->message.payload))
        {
            hal_gpio_output_high(&gpio);
        }
        else
        {
            hal_gpio_output_low(&gpio);
        }
    }
    else
    {
        LOG("Invalid type %d", link->message.type);
    }
    IOTLINK_FREE_MESSAGE(&(link->message));
    // LOG("targetGpio end");
}