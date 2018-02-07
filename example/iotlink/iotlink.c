#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>
#include <factory.h>

#if defined(IOT_LINK_WIFI)
#include <netmgr.h>
#endif

static void runLink(input_event_t *event, void *arg)
{
    LINK *link = arg;
    LOG("runLink triggerred with running:%d, readFunc pointer:%p and writeFunc pointer:%p", link->running, link->readFunc, link->writeFunc);
    if (link->readFunc != NULL && link->writeFunc != NULL)
    {
        if (!link->running)
        {
            LOG("Start link");
            link->running = true;
            (*(link->readFunc))(link);
        }
        else
        {
            LOG("Link is running already");
        }
    }
    else
    {
        LOG("Link not ready");
    }
}

int application_start(int argc, char *argv[])
{
    aos_set_log_level(AOS_LL_DEBUG);
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"test\",\"password\":\"12345678\"},\"links\":[{\"source\":{\"module\":\"dht11Source\",\"options\":{\"pin\":4,\"interval\":10000}},\"target\":{\"module\":\"MqttTarget\",\"options\":{\"topic\":\"topic1\",\"host\":\"mqtt\"}}}]}");
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"Xiaomi_5576\",\"password\":\"\"},\"transport\":{\"type\":\"mqtt\",\"host\":\"m2m.eclipse.org\",\"port\":1883,\"username\":\"abcdefgh\",\"password\":\"asdfsdfd\",\"clientId\":\"iotlink001\",\"pubkey\":null},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"mqtt\",\"topic\":\"iotlink\"}}]}");
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"Xiaomi_5576\",\"password\":\"w19l86s07\"},\"transport\":{\"type\":\"mqtt\",\"host\":\"mqtt.pndsn.com\",\"port\":1883,\"username\":\"iotlink\",\"password\":\"iotlink\",\"clientId\":\"pub-c-bc9c7186-ff77-4968-9004-be75eeaaeffb/sub-c-66fa562c-849f-11e7-aa94-3ef20c3716d4/mib002\",\"pubkey\":null},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":1},\"target\":{\"type\":\"mqtt\",\"topic\":\"iotlink\"}},{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"dummy\"}}]}");
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"Xiaomi_5576\",\"password\":\"w19l86s07\"},\"transport\":{\"type\":\"mqtt\",\"host\":\"mqtt.pndsn.com\",\"port\":1883,\"username\":\"iotlink\",\"password\":\"iotlink\",\"clientId\":\"pub-c-bc9c7186-ff77-4968-9004-be75eeaaeffb/sub-c-66fa562c-849f-11e7-aa94-3ef20c3716d4/mib002\",\"pubkey\":null},\"links\":[{\"source\":{\"type\":\"irq\",\"port\":15},\"target\":{\"type\":\"dummy\"}}]}");
    cJSON *root = cJSON_Parse("{\"links\":[{\"source\":{\"type\":\"dht11\",\"port\":16,\"interval\":1000},\"target\":{\"type\":\"dummy\"}}]}");
    LOG("Parsed config json");
    cJSON *linksConfig = jsonObj(root, "links");
    cJSON *linkConfig;
    TRANSPORT *transport = createTransports(jsonObj(root, "transport"));
    LOG("Transport created");
    for (linkConfig = (linksConfig != NULL) ? (linksConfig)->child : NULL; linkConfig != NULL; linkConfig = linkConfig->next)
    {
        LOG("Creating link");
        LINK *link = createLink(linkConfig);
        link->transport = transport;
        aos_register_event_filter(EV_LINK_UPDATED, runLink, link);
        LOG("Link created");
    }

#if defined(IOT_LINK_WIFI)
    cJSON *wifi = jsonObj(root, "wifi");
    if (wifi != NULL)
    {
        netmgr_init();
        netmgr_ap_config_t netmgrConfig;
        memset(&netmgrConfig, 0, sizeof(netmgrConfig));
        strcpy(netmgrConfig.ssid, jsonStr(wifi, "ssid"));
        strcpy(netmgrConfig.pwd, jsonStr(wifi, "password"));
        netmgr_set_ap_config(&netmgrConfig);
        netmgr_start(false);
        LOG("netmgr started");
    }
#endif

    aos_loop_run();

    return 0;
}
