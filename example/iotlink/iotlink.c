#include <aos/aos.h>
#include <cJSON.h>
#include <factory.h>

static void runLink(void * arg)
{
    LINK *link = arg;
    (*(link->readFunc))(link);
}

int application_start(int argc, char *argv[])
{
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"test\",\"password\":\"12345678\"},\"links\":[{\"source\":{\"module\":\"dht11Source\",\"options\":{\"pin\":4,\"interval\":10000}},\"target\":{\"module\":\"MqttTarget\",\"options\":{\"topic\":\"topic1\",\"host\":\"mqtt\"}}}]}");
    cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"test\",\"password\":\"12345678\"},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"dummy\"}}]}");
    const cJSON *linksConfig = cJSON_GetObjectItem(root, "links");
    const cJSON *linkConfig;
    for(linkConfig = (linksConfig != NULL) ? (linksConfig)->child : NULL; linkConfig != NULL; linkConfig = linkConfig->next)
    {
        LINK *link = iotlink_createLink(linkConfig);
        aos_post_delayed_action(0, runLink, link);
    }
    aos_loop_run();

    return 0;
}

