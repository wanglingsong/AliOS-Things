#include <stdlib.h>
#include <aos/aos.h>
#include <hal/soc/soc.h>

#include "dht11.h"

#define DHT11_READ_MAX_TIME 85

bool dht11_read(DHT11 *ptdht11, gpio_dev_t *gpio)
{

    int32_t lststate = 1;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    int dht11_val[5] = {0, 0, 0, 0, 0};
    int32_t ret;

    //    pinMode(DHT11PIN, OUTPUT);
    gpio->config = OUTPUT_OPEN_DRAIN_PULL_UP;
    ret = hal_gpio_init(gpio);
    LOG("gpio output init? %d", ret);
    //    digitalWrite(DHT11PIN, LOW);
    ret = hal_gpio_output_low(gpio);
    LOG("gpio output low? %d", ret);
    aos_msleep(18);
    //    digitalWrite(DHT11PIN, HIGH);
    ret = hal_gpio_output_high(gpio);
    LOG("gpio output high? %d", ret);
    aos_msleep(40);
    // ret = hal_gpio_finalize(gpio);
    // LOG("gpio output finalized? %d", ret);
    //    pinMode(DHT11PIN, INPUT);
    gpio->config = INPUT_PULL_UP;
    ret = hal_gpio_init(gpio);
    LOG("gpio input init? %d", ret);

    for (i = 0; i < DHT11_READ_MAX_TIME; i++)
    {

        counter = 0;
        uint32_t value = 0;
        ret = hal_gpio_input_get(gpio, &value);
        LOG("gpio input get? %d with value: %d", ret, value);

        while (value == lststate)
        {
            counter++;
            aos_msleep(1);
            if (counter == 255)
            {
                LOG("Reach time limit");
                break;
            }
            ret = hal_gpio_input_get(gpio, &value);
            LOG("gpio input get? %d with value: %d", ret, value);
        }

        if (counter == 255)
        {
            LOG("Reach time limit");
            break;
        }

        lststate = value;


        // top 3 transistions are ignored

        if ((i >= 4) && (i % 2 == 0))
        {

            dht11_val[j / 8] <<= 1;

            if (counter > 16)
                dht11_val[j / 8] |= 1;

            j++;
        }
    }

    ret = hal_gpio_finalize(gpio);
    LOG("gpio input finalized? %d", ret);

    for (int i = 0; i < 5; i++)
    {
        LOG("%d ", dht11_val[i]);
    }

    // verify cheksum and print the verified data

    if ((j >= 40) && (dht11_val[4] == ((dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3]) & 0xFF)))
    {

        if ((dht11_val[0] == 0) && (dht11_val[2] == 0))
        {
            return false;
        }

        ptdht11->temperature = (uint16_t)(dht11_val[2] + dht11_val[3] / 10);
        ptdht11->humidity = (uint16_t)(dht11_val[0] + dht11_val[1] / 10);

        return true;
    }
    return false;
}