#include <aos/aos.h>
// TODO
#include "rom/ets_sys.h"
#include <hal/soc/gpio.h>
#include "dht11.h"

#define DHT11_TIMEOUT_ERROR_STAGE_1 -101
#define DHT11_TIMEOUT_ERROR_STAGE_2 -102
#define DHT11_TIMEOUT_ERROR_STAGE_3 -103
#define DHT11_TIMEOUT_ERROR_STAGE_4 -104
#define DHT11_TIMEOUT_ERROR_STAGE_5 -105
#define DHT11_TIMEOUT_ERROR_STAGE_6 -106

int32_t dht11_read(DHT11 *ptdht11, uint8_t pin)
{

    uint8_t counter = 0;
    uint8_t byteCounter = 0;
    uint8_t bits[5] = {0, 0, 0, 0, 0};
    uint8_t cnt = 7;
    uint32_t value = 0;
    int32_t ret = 0;

    gpio_dev_t gpio;
    gpio.port = pin;

    gpio.config = OUTPUT_PUSH_PULL;
    ret = hal_gpio_init(&gpio);
    // LOG("gpio output init? %d", ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = hal_gpio_output_low(&gpio);
    // LOG("gpio output low? %d", ret);
    if (ret != 0)
    {
        return ret;
    }
    // TODO
    ets_delay_us(22000);
    ret = hal_gpio_output_high(&gpio);
    // LOG("gpio output high? %d", ret);
    if (ret != 0)
    {
        return ret;
    }
    // ets_delay_us(43);
    gpio.config = INPUT_PULL_UP;
    ret = hal_gpio_init(&gpio);
    if (ret != 0)
    {
        return ret;
    }
    ret = hal_gpio_input_get(&gpio, &value);
    if (ret != 0)
    {
        return ret;
    }

    while (value == 1)
    {
        if (counter > 40)
        {
            return DHT11_TIMEOUT_ERROR_STAGE_1;
        }
        counter += 10;
        ets_delay_us(10);
        ret = hal_gpio_input_get(&gpio, &value);
        if (ret != 0)
        {
            return ret;
        }
    }
    //Now that the DHT has pulled the line low,
    //it will keep the line low for 80 us and then high for 80us
    //check to see if it keeps low
    counter = 0;
    while (value == 0)
    {
        if (counter > 80)
        {
            return DHT11_TIMEOUT_ERROR_STAGE_2;
        }
        counter += 10;
        ets_delay_us(10);
        ret = hal_gpio_input_get(&gpio, &value);
        if (ret != 0)
        {
            return ret;
        }
    }
    counter = 0;
    while (value == 1)
    {
        if (counter > 80)
        {
            return DHT11_TIMEOUT_ERROR_STAGE_3;
        }
        counter += 10;
        ets_delay_us(10);
        ret = hal_gpio_input_get(&gpio, &value);
        if (ret != 0)
        {
            return ret;
        }
    }
    // If no errors have occurred, it is time to read data
    //output data from the DHT11 is 40 bits.
    //Loop here until 40 bits have been read or a timeout occurs

    for (int i = 0; i < 40; i++)
    {
        //int currentBit = 0;
        //starts new data transmission with 50us low signal
        counter = 0;
        while (value == 0)
        {
            if (counter > 60)
            {
                return DHT11_TIMEOUT_ERROR_STAGE_4;
            }
            counter += 10;
            ets_delay_us(10);
            ret = hal_gpio_input_get(&gpio, &value);
            if (ret != 0)
            {
                return ret;
            }
        }

        //Now check to see if new data is a 0 or a 1
        counter = 0;
        while (value == 1)
        {
            if (counter > 80)
            {
                return DHT11_TIMEOUT_ERROR_STAGE_5;
            }
            counter += 10;
            ets_delay_us(10);
            ret = hal_gpio_input_get(&gpio, &value);
            if (ret != 0)
            {
                return ret;
            }
        }

        //add the current reading to the output data
        //since all bits where set to 0 at the start of the loop, only looking for 1s
        //look for when count is greater than 40 - this allows for some margin of error
        if (counter > 40)
        {
            bits[byteCounter] |= (1 << cnt);
        }
        //here are conditionals that work with the bit counters
        if (cnt == 0)
        {
            cnt = 7;
            byteCounter = byteCounter + 1;
        }
        else
        {
            cnt = cnt - 1;
        }
    }

    uint8_t sum = bits[0] + bits[2];

    if (bits[4] != sum)
    {
        return DHT11_TIMEOUT_ERROR_STAGE_6;
    }
    ptdht11->humidity = bits[0];
    ptdht11->temperature = bits[2];

    return 0;
}