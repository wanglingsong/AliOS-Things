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
        else if (strcmp(cmd, "update-config") == 0)
        {
            char *config = jsonStr(json, "config");
            aos_kv_set("iotlink_config", config, strlen(config), 1);
            LOG("Saved config to KV from cli target: %s", config);
        }
        else
        {
            LOG("Invalid cmd %s", cmd);
        }
    }
    IOTLINK_FREE_MESSAGE(&(link->message));
}