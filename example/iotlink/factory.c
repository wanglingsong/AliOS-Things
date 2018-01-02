#include <aos/aos.h>
#include <factory.h>
#include <string.h>

static cJSON* jsonObj(const cJSON *json, char *key)
{
    return cJSON_GetObjectItemCaseSensitive(json, key);
}

static char* jsonStr(const cJSON *json, char *key)
{
    return jsonObj(json, key)->valuestring;
}

static int jsonInt(const cJSON *json, char *key)
{
    return jsonObj(json, key)->valueint;
}

static void readFromDummySource(void * link)
{
    LINK* linkp = link;
    linkp->payload = cJSON_Parse("{\"source\":\"dummy\"}");
    aos_post_delayed_action(0, linkp->writeFunc, link);
    int interval = jsonInt(linkp->sourceConfig, "interval");
    if (interval > 0) {
        aos_post_delayed_action(interval, readFromDummySource, link);
    }
}

static void writeToDummyTarget(void * link)
{
    LINK* linkp = link;
    char *str = cJSON_Print(linkp->payload);
    LOG("Dummy Target received: %s\r\n", str);
    aos_free(str);
    cJSON_Delete(linkp->payload);
    linkp->payload = NULL;
}

static void readFromButtonSource(void * link)
{
    // TODO
}

static void writeToLedTarget(void * link)
{
    // TODO
}

static void readFromMqttSource(void * link)
{
    // TODO
}

static void writeToMqttTarget(void * link)
{
    // TODO
}

static void createSource(LINK * link, const cJSON * config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->readFunc = readFromDummySource;
    } else {
        // TODO
    }
}

static void createTarget(LINK * link, const cJSON * config)
{
    char *type = jsonStr(config, "type");
    if (strcmp(type, "dummy") == 0) {
        link->writeFunc = writeToDummyTarget;
    } else {
        // TODO
    }
}

LINK* iotlink_createLink(const cJSON * config)
{
    LINK *link = aos_malloc(sizeof(LINK));
    cJSON *sourceConfig = jsonObj(config, "source");
    link->sourceConfig = sourceConfig;
    cJSON *targetConfig = jsonObj(config, "target");
    link->targetConfig = targetConfig;
    createSource(link, sourceConfig);
    createTarget(link, targetConfig);
    return link;
}