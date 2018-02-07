NAME := iotlink

$(NAME)_SOURCES := util.c mqtt.c dummy.c gpio.c drivers/dht11.c dht.c factory.c iotlink.c

# GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_DEFINES += IOT_LINK_WIFI IOT_LINK_MQTT

$(NAME)_COMPONENTS := yloop cli cjson connectivity.mqtt netmgr

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif