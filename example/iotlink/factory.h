#include <cjson/cJSON.h>

typedef void (*FUNC)(void*);

typedef struct LINK
{
    cJSON *sourceConfig;
    FUNC readFunc;
    cJSON *targetConfig;
    FUNC writeFunc;
    void *payload;
} LINK;

LINK* iotlink_createLink(const cJSON * config);