#define  EV_MQTT_CONNETED                  0x1001
#define  EV_MQTT_DISCONNETED               0x1002
#define  EV_LINK_UPDATED                   0x1003

typedef void (*FUNC)(void*);

typedef struct TRANSPORT
{
    cJSON *config;
    void *client;
} TRANSPORT;

typedef struct LINK
{
    cJSON *sourceConfig;
    FUNC readFunc;
    cJSON *targetConfig;
    FUNC writeFunc;
    void *payload;
    TRANSPORT *transport;
    bool running;
} LINK;