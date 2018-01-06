#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <util.h>
#include <types.h>
#include <factory.h>

#if defined(IOT_LINK_WIFI)
#include <netmgr.h>
#endif

static void runLink(input_event_t *event, void * arg)
{
    LINK *link = arg;
    if (link->readFunc != NULL && link->writeFunc != NULL) {
        (*(link->readFunc))(link);
    } else {
        // TODO
    }
}

int application_start(int argc, char *argv[])
{
    // cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"test\",\"password\":\"12345678\"},\"links\":[{\"source\":{\"module\":\"dht11Source\",\"options\":{\"pin\":4,\"interval\":10000}},\"target\":{\"module\":\"MqttTarget\",\"options\":{\"topic\":\"topic1\",\"host\":\"mqtt\"}}}]}");
    cJSON *root = cJSON_Parse("{\"wifi\":{\"ssid\":\"Xiaomi_5576\",\"password\":\"w86l07s13\"},\"transport\":{\"type\":\"mqtt\"},\"links\":[{\"source\":{\"type\":\"dummy\",\"interval\":5000},\"target\":{\"type\":\"dummy\"}}]}");
    cJSON *linksConfig = cJSON_GetObjectItem(root, "links");
    cJSON *linkConfig;
    TRANSPORT *transport = iotlink_createTransports(jsonObj(root, "transport"));
    for(linkConfig = (linksConfig != NULL) ? (linksConfig)->child : NULL; linkConfig != NULL; linkConfig = linkConfig->next)
    {
        LINK *link = iotlink_createLink(linkConfig);
        link->transport = transport;
        aos_register_event_filter(EV_LINK_UPDATED, runLink, link);
    }

#if defined(IOT_LINK_WIFI)
    // TODO wifi and create transport
    // aos_set_log_level(AOS_LL_INFO);
    cJSON *wifi = cJSON_GetObjectItem(root, "wifi");
	netmgr_init();
    netmgr_ap_config_t netmgrConfig;
    memset(&netmgrConfig, 0, sizeof(netmgrConfig));
    strcpy(netmgrConfig.ssid, jsonStr(wifi, "ssid"));
    strcpy(netmgrConfig.pwd, jsonStr(wifi, "password"));
    netmgr_set_ap_config(&netmgrConfig);
	netmgr_start(false);
#endif

    aos_loop_run();

    return 0;
}

