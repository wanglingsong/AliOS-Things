/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "rom/ets_sys.h"

extern void esp_restart();

void hal_reboot(void)
{
    printf("reboot!\n");
    esp_restart();
}

void hal_delay_us(uint32_t us)
{
    ets_delay_us(us);
}
