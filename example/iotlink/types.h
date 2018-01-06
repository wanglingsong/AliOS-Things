// #include <cJSON.h>

#define  EV_MQTT_CONNETED                  0x1001
#define  EV_MQTT_DISCONNETED               0x1002
#define  EV_LINK_UPDATED                   0x1003

typedef void (*FUNC)(void*);
// typedef void (*PUB_FUNC)(void *, void*);

typedef struct TRANSPORT
{
    cJSON *config;
    void *client;
    // PUB_FUNC publishFunc;
} TRANSPORT;

typedef struct LINK
{
    cJSON *sourceConfig;
    FUNC readFunc;
    // TODO FUNC readStop;
    cJSON *targetConfig;
    FUNC writeFunc;
    // TODO FUNC writeStop;
    void *payload;
    TRANSPORT *transport;
} LINK;