#include <stdlib.h>
#include <aos/aos.h>
// TODO
#include "rom/ets_sys.h"

#include <hal/soc/gpio.h>

#include "dht11.h"

#define DHT11_READ_MAX_TIME 85

bool dht11_read(DHT11 *ptdht11, int pin)
{

    uint8_t counter = 0;
    uint8_t byteCounter = 0;
    uint8_t bits[5] = {0, 0, 0, 0, 0};
    uint8_t cnt = 7;
    int value = 0;

    gpio_dev_t gpio;
    gpio.port = pin;

    gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&gpio);
    // LOG("gpio output init? %d", ret);
    hal_gpio_output_low(&gpio);
    // LOG("gpio output low? %d", ret);
    ets_delay_us(22000);
    hal_gpio_output_high(&gpio);
    // LOG("gpio output high? %d", ret);
    ets_delay_us(43);
    gpio.config = INPUT_PULL_UP;
    hal_gpio_init(&gpio);
    hal_gpio_input_get(&gpio, &value);

    while (value == 1)
    {
        if (counter > 40)
        {
            LOG("E1");
            return false;
        }
        counter = counter + 1;
        ets_delay_us(1);
        hal_gpio_input_get(&gpio, &value);
    }
    //Now that the DHT has pulled the line low,
    //it will keep the line low for 80 us and then high for 80us
    //check to see if it keeps low
    counter = 0;
    while (value == 0)
    {
        if (counter > 80)
        {
            LOG("E2");
            return false;
        }
        counter = counter + 1;
        ets_delay_us(1);
        hal_gpio_input_get(&gpio, &value);
    }
    counter = 0;
    while (value == 1)
    {
        if (counter > 80)
        {
            LOG("E3");
            return false;
        }
        counter = counter + 1;
        ets_delay_us(1);
        hal_gpio_input_get(&gpio, &value);
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
                LOG("E4");
                return false;
            }
            counter = counter + 10;
            ets_delay_us(10);
            hal_gpio_input_get(&gpio, &value);
        }

        //Now check to see if new data is a 0 or a 1
        counter = 0;
        while (value == 1)
        {
            if (counter > 75)
            {
                LOG("counter: %d", counter);
                LOG("E5");
                return false;
            }
            counter = counter + 10;
            ets_delay_us(10);
            hal_gpio_input_get(&gpio, &value);
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
        LOG("E6");
        return false;
    }
    ptdht11->humidity = bits[0];
    ptdht11->temperature = bits[2];

    return true;
}