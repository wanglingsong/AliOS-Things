#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <util.h>
#include <types.h>
#include <factory.h>

#if defined(IOT_LINK_WIFI)
#include <netmgr.h>
#endif

static void runLink(input_event_t *event, void *arg)
{
    LINK *link = arg;
    LOG("runLink triggerred with running:%d, readFunc pointer:%p and writeFunc pointer:%p", link->running, link->readFunc, link->writeFunc);
    if (link->readFunc != NULL && link->writeFunc != NULL) {
        if (!link->running) {
            LOG("Start link");
            link->running = true;
            (*(link->readFunc))(link);
        } else {
            LOG("Skip link");
        }
    } else {
        LOG("Link not ready");
    }
}

int application_start(int argc, char *argv[])
{
    aos_set_log_level(AOS_LL_DEBUG);
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"test\",\"password\":\"12345678\"},\"links\":[{\"source\":{\"module\":\"dht11Source\",\"options\":{\"pin\":4,\"interval\":10000}},\"target\":{\"module\":\"MqttTarget\",\"options\":{\"topic\":\"topic1\",\"host\":\"mqtt\"}}}]}");
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"TP-LINK_7B4CC6\",\"password\":\"\"},\"transport\":{\"type\":\"mqtt\",\"host\":\"m2m.eclipse.org\",\"port\":1883,\"username\":\"iotlink\",\"password\":\"iotlink\",\"clientId\":\"iotlink001\",\"pubkey\":null},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"dummy\"}}]}");
    cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"TP-LINK_7B4CC6\",\"password\":\"\"},\"transport\":{\"type\":\"mqtt\",\"host\":\"m2m.eclipse.org\",\"port\":1883,\"username\":\"iotlink\",\"password\":\"iotlink\",\"clientId\":\"iotlink001\",\"pubkey\":null},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"mqtt\",\"topic\":\"iotilnk-target\"}}]}");
    LOG("Parsed config json");
    cJSON *linksConfig = jsonObj(root, "links");
    cJSON *linkConfig;
    TRANSPORT *transport = createTransports(jsonObj(root, "transport"));
    LOG("Transport created");
    for(linkConfig = (linksConfig != NULL) ? (linksConfig)->child : NULL; linkConfig != NULL; linkConfig = linkConfig->next)
    {
        LOG("Creating link");
        LINK *link = createLink(linkConfig);
        link->transport = transport;
        aos_register_event_filter(EV_LINK_UPDATED, runLink, link);
        LOG("Link created");
    }

#if defined(IOT_LINK_WIFI)
    cJSON *wifi = jsonObj(root, "wifi");
	netmgr_init();
    netmgr_ap_config_t netmgrConfig;
    memset(&netmgrConfig, 0, sizeof(netmgrConfig));
    strcpy(netmgrConfig.ssid, jsonStr(wifi, "ssid"));
    strcpy(netmgrConfig.pwd, jsonStr(wifi, "password"));
    netmgr_set_ap_config(&netmgrConfig);
	netmgr_start(false);
    LOG("netmgr started");
#endif

    aos_loop_run();

    return 0;
}

