#ifndef DHT11_H
#define DHT11_H

typedef struct
{
	uint16_t humidity;
	uint16_t temperature;
} DHT11;

int32_t dht11_read(DHT11 *ptdht11, uint8_t port);

#endif
