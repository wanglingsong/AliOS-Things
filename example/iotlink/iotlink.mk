NAME := iotlink

$(NAME)_SOURCES := util.c factory.c iotlink.c
# mqtt.c 

# GLOBAL_DEFINES += AOS_NO_WIFI

$(NAME)_COMPONENTS := yloop cli cjson 
# connectivity.mqtt netmgr

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif

