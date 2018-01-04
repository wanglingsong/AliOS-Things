NAME := iotlink

$(NAME)_SOURCES := factory.c iotlink.c

GLOBAL_DEFINES += AOS_NO_WIFI

$(NAME)_COMPONENTS += yloop cli cjson

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif

