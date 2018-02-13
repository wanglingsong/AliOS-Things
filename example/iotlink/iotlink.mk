NAME := iotlink

$(NAME)_SOURCES := util.c pubnub_msg_adaptor.c sensors/dht11.c modules/mqtt.c modules/dht.c modules/dummy.c modules/gpio.c modules/ir.c modules/cli.c factory.c iotlink.c

# GLOBAL_DEFINES += AOS_NO_WIFI
GLOBAL_DEFINES += IOT_LINK_WIFI IOT_LINK_MQTT

$(NAME)_COMPONENTS := yloop cli cjson connectivity.mqtt netmgr

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif