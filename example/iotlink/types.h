#define EV_MQTT_CONNETED 0x1001
#define EV_MQTT_DISCONNETED 0x1002
#define EV_LINK_UPDATED 0x1003

typedef enum {
    MESSAGE_SOURCE_DUMMY = 0,
    MESSAGE_SOURCE_GPIO_TRIGGER,
    MESSAGE_SOURCE_DHT11,
    MESSAGE_SOURCE_MQTT
} MESSAGE_SOURCE;

typedef enum {
    MESSAGE_TYPE_BOOLEAN = 0,
    MESSAGE_TYPE_NUMBER,
    MESSAGE_TYPE_STRING,
    MESSAGE_TYPE_JSON
} MESSAGE_TYPE;

typedef struct IOTLINK_MESSAGE
{
    MESSAGE_SOURCE source;
    MESSAGE_TYPE type;
    void *payload;
} IOTLINK_MESSAGE;

typedef void (*FUNC)(void *);

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
    IOTLINK_MESSAGE message;
    TRANSPORT *transport;
    bool running;
} LINK;