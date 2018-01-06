#include <string.h>
#include <aos/aos.h>
#include <netmgr.h>
#include "iot_import.h"
#include "iot_export.h"
#include <cJSON.h>
#include <types.h>
#include <util.h>

#define MSG_LEN_MAX             (2048)
#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        printf("%s|%03d :: ", __func__, __LINE__); \
        printf(fmt, ##args); \
        printf("%s", "\r\n"); \
    } while(0)

static void *gpclient;

static char *msg_buf = NULL, *msg_readbuf = NULL;

static void release_buff() {
    if (NULL != msg_buf) {
        aos_free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        aos_free(msg_readbuf);
    }
}

static void event_handle_mqtt(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            aos_post_event(EV_MQTT_DISCONNETED, 0, 0);
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            aos_post_event(EV_MQTT_CONNETED, 0, 0);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static int createMqttClient(void *arg)
{
    int rc = 0;
    // iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;

    if (msg_buf != NULL) {
        return rc;
    }

    if (NULL == (msg_buf = (char *)aos_malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        release_buff();
        return rc;
    }

    if (NULL == (msg_readbuf = (char *)aos_malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        release_buff();
        return rc;
    }

    // /* Device AUTH */
    // if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
    //     EXAMPLE_TRACE("AUTH request failed!");
    //     rc = -1;
    //     release_buff();
    //     return rc;
    // }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    TRANSPORT *transport = arg;
    mqtt_params.port = 1883;
    mqtt_params.host = "iot.eclipse.org";
    // mqtt_params.host = jsonStr(transport->config, "host");
    mqtt_params.client_id = "iotlink_001";
    mqtt_params.username = NULL;
    mqtt_params.password = NULL;
    mqtt_params.pub_key = NULL;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle_mqtt;
    mqtt_params.handle_event.pcontext = arg;

    /* Construct a MQTT client with specify parameter */
    gpclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == gpclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        release_buff();
    } else {
        transport->client = gpclient;
    }

    return rc;
}

static void wifi_service_event(input_event_t *event, void *priv_data) {
    EXAMPLE_TRACE("wifi_service_event!");
    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    if (NULL != gpclient) {
        createMqttClient(priv_data);
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    LINK *link = pcontext;
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    // print topic name and topic message
    // EXAMPLE_TRACE("----");
    // EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
    //               ptopic_info->topic_len,
    //               ptopic_info->ptopic,
    //               ptopic_info->topic_len);
    // EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
    //               ptopic_info->payload_len,
    //               ptopic_info->payload,
    //               ptopic_info->payload_len);
    // EXAMPLE_TRACE("----");
    // g_led_flag = 0;
    // if (strstr((char *) ptopic_info->payload, "\"desired\":{\"LED_value\":\"On\"}") != NULL) {
    //     g_led_flag = 1;
    // } else if (strstr((char *) ptopic_info->payload, "\"desired\":{\"LED_value\":\"Off\"}") != NULL) {
    //     g_led_flag = 2;
    // }

    cJSON *payload = cJSON_CreateObject();
    cJSON_AddItemToObject(payload, "source", cJSON_CreateString("mqtt"));
    cJSON_AddItemToObject(payload, "type", cJSON_CreateString("string"));
    cJSON_AddItemToObject(payload, "payload", cJSON_CreateString(ptopic_info->payload));
    link->payload = payload;
    aos_post_delayed_action(0, link->writeFunc, link);
}

void readFromMqttSource(void *arg)
{
    LINK *link = arg;
    // aos_register_event_filter(EV_WIFI, wifi_service_event, link);
    // TODO
    if (NULL == (link->transport->client)) {

    } else {
        IOT_MQTT_Subscribe(link->transport->client, jsonStr(link->sourceConfig, "topic"), IOTX_MQTT_QOS1, _demo_message_arrive, link);
    }
}

static void publish(void *arg)
{
    LINK *link = arg;
    char *msg = cJSON_Print(link->payload);
    iotx_mqtt_topic_info_t *topic_msg = aos_malloc(sizeof(iotx_mqtt_topic_info_t));
    topic_msg->qos = IOTX_MQTT_QOS1;
    topic_msg->retain = 0;
    topic_msg->dup = 0;
    topic_msg->payload = (void *)msg;
    topic_msg->payload_len = strlen(msg);

    IOT_MQTT_Publish(link->transport->client, jsonStr(link->targetConfig, "topic"), topic_msg);

    aos_free(topic_msg);
    aos_free(msg);
}

void writeToMqttTarget(void *arg)
{
    LINK* link = arg;
    if (NULL == link->transport->client) {
        // aos_register_event_filter(EV_WIFI, wifi_service_event, link);
    } else {
        publish(link);
        cJSON_Delete(link->payload);
        link->payload = NULL;
    }
    // TODO
}

TRANSPORT* createMqttTransport(cJSON *config)
{
    TRANSPORT *transport = aos_malloc(sizeof(TRANSPORT));
    transport->config = config;
    aos_register_event_filter(EV_WIFI, wifi_service_event, transport);
    return transport;
}