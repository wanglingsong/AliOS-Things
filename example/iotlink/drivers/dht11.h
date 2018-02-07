#ifndef DHT11_H
#define DHT11_H


#include <stdint.h>


typedef struct {
	uint16_t humidity;
	uint16_t temperature;
} DHT11;


bool dht11_read(DHT11 *ptdht11, gpio_dev_t *gpio);

#endif
