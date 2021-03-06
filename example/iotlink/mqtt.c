#include <string.h>
#include <aos/aos.h>
#include <netmgr.h>
#include "iot_import.h"
#include "iot_export.h"
#include <cJSON.h>
#include <types.h>
#include <util.h>
// #include "device.h"

#define MSG_LEN_MAX             (2048)

#define PRODUCT_KEY             "BfKxBDSjWCH"
#define DEVICE_NAME             "aos_mqtt_test"
#define DEVICE_SECRET           "zcBZ5TB9cfAylUGo1flH0o47PxS8Mqu2"

// TODO move these two buffer into context
static char *msg_buf = NULL, *msg_readbuf = NULL;

static void release_buff() {
    if (NULL != msg_buf) {
        aos_free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        aos_free(msg_readbuf);
    }
}

static void eventHandleMqtt(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            LOG("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            LOG("MQTT disconnect.");
            aos_post_event(EV_MQTT_DISCONNETED, 0, 0);
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            LOG("MQTT reconnect.");
            aos_post_event(EV_MQTT_CONNETED, 0, 0);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            LOG("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            LOG("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            LOG("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            LOG("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            LOG("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            LOG("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            LOG("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            LOG("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            LOG("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            LOG("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            LOG("Should NOT arrive here.");
            break;
    }
}

static void mqtt_service_event(input_event_t *event, void *priv_data) {
    LOG("wifi_service_event!");
    if (event->type != EV_SYS) {
        return;
    }

    if (event->code != CODE_SYS_ON_MQTT_READ) {
        return;
    }
    LOG("EV_MQTT_CONNETED received");
    aos_post_event(EV_MQTT_CONNETED, 0, 0);
}

static int createMqttClient(void *arg)
{
    int rc = 0;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;

    if (msg_buf != NULL) {
        return rc;
    }

    if (NULL == (msg_buf = (char *)aos_malloc(MSG_LEN_MAX))) {
        LOG("not enough memory");
        rc = -1;
        release_buff();
        return rc;
    }

    if (NULL == (msg_readbuf = (char *)aos_malloc(MSG_LEN_MAX))) {
        LOG("not enough memory");
        rc = -1;
        release_buff();
        return rc;
    }

    // TODO mistery code!
    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        // EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        release_buff();
        return rc;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    TRANSPORT *transport = arg;
    mqtt_params.port = jsonInt(transport->config, "port");
    mqtt_params.host = jsonStr(transport->config, "host");
    mqtt_params.client_id = jsonStr(transport->config, "clientId");
    mqtt_params.username = jsonStr(transport->config, "username");
    mqtt_params.password = jsonStr(transport->config, "password");
    mqtt_params.pub_key = jsonStr(transport->config, "pubkey");

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = eventHandleMqtt;
    mqtt_params.handle_event.pcontext = arg;
    /* Construct a MQTT client with specify parameter */
    void *gpclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == gpclient) {
        LOG("MQTT construct failed");
        rc = -1;
        release_buff();
    } else {
        transport->client = gpclient;
        LOG("MQTT construct success");
        aos_register_event_filter(EV_SYS,  mqtt_service_event, arg);
    }
    return rc;
}

static void wifiServiceEvent(input_event_t *event, void *priv_data) {
    LOG("wifi_service_event with type:%d and code:%d", event->type, event->code);
    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }
    TRANSPORT *transport = priv_data;

    if (NULL == transport->client) {
        if (createMqttClient(priv_data) == 0) {
            LOG("MQTT client created");
        } else {
            LOG("Failed to create MQTT client");
            // TODO reconnect
        }
    }
}

static void subscibedMessageArrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    LINK *link = pcontext;
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    LOG("Topic:%s Payload:%s",
                  ptopic_info->ptopic,
                  ptopic_info->payload);
    if (!link->running) {
        IOT_MQTT_Unsubscribe(pclient, jsonStr(link->sourceConfig, "topic"));
        return;
    }
    // cJSON *payload = cJSON_CreateObject();
    // cJSON_AddItemToObject(payload, "source", cJSON_CreateString("mqtt"));
    // cJSON_AddItemToObject(payload, "type", cJSON_CreateString("string"));
    // cJSON_AddItemToObject(payload, "payload", cJSON_CreateString(ptopic_info->payload));
    // link->payload = payload;
    IOTLINK_MESSAGE *message = aos_malloc(sizeof(IOTLINK_MESSAGE));
    message->source = MESSAGE_SOURCE_MQTT;
    message->type = MESSAGE_TYPE_STRING;
    message->payload = "It's dummy message";
    link->message = (void*)(ptopic_info->payload);
    aos_post_delayed_action(0, link->writeFunc, link);
}

void sourceMqtt(void *arg)
{
    LINK *link = arg;
    if (NULL == (link->transport->client)) {
        LOG("Not ready to read from MQTT source");
    } else {
        IOT_MQTT_Subscribe(link->transport->client, jsonStr(link->sourceConfig, "topic"), IOTX_MQTT_QOS1, subscibedMessageArrive, link);
    }
}

static void publishMqttMessage(void *arg)
{
    LINK *link = arg;
    char *msg = IOTLINK_PRINT_MESSAGE(link->message);
    iotx_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(topic_msg));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)(msg);
    topic_msg.payload_len = strlen(msg);
    char *topic = jsonStr(link->targetConfig, "topic");
    int rc = IOT_MQTT_Publish(link->transport->client, topic, &topic_msg);
    if (rc < 0) {
        LOG("Failed to publish mqtt message:%s to topic: %s with return code:%d", msg, topic, rc);
    } else {
        LOG("Published mqtt message:%s to topic: %s", msg, topic);
    }
    aos_free(msg);
    IOTLINK_FREE_MESSAGE(link->message);
    link->message = NULL;
}

void targetMqtt(void *arg)
{
    LINK* link = arg;
    if (NULL == link->transport->client) {
        LOG("Not ready to write to MQTT target");
    } else {
        publishMqttMessage(link);
    }
}

TRANSPORT* createMqttTransport(cJSON *config)
{
    TRANSPORT *transport = aos_zalloc(sizeof(TRANSPORT));
    transport->config = config;
    aos_register_event_filter(EV_WIFI, wifiServiceEvent, transport);
    return transport;
}