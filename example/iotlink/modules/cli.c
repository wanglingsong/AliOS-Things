#include <string.h>
#include <aos/aos.h>
#include <cJSON.h>
#include <types.h>
#include <util.h>

extern void hal_reboot(void);

void targetCli(void *arg)
{
    LINK *link = arg;
    if (link->message.type == MESSAGE_TYPE_JSON)
    {
        cJSON *json = link->message.payload;
        char *cmd = jsonStr(json, "cmd");
        if (strcmp(cmd, "reboot") == 0)
        {
            hal_reboot();
        }
        else if (strcmp(cmd, "set-kv") == 0)
        {
            // TODO
            char *key = jsonStr(json, "key");
            char *value = jsonStr(json, "value");
            aos_kv_set(key, value, strlen(value), 1);
            LOG("Saved key:%s value:%s to KV from cli target", key, value);
        }
        else
        {
            LOG("Invalid cmd %s", cmd);
        }
    }
    IOTLINK_FREE_MESSAGE(&(link->message));
}