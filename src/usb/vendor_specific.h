// SPDX-FileCopyrightText: 2024 ShinagwaKazemaru
// SPDX-License-Identifier: MIT

/**
 *   This file is based on
 *   pico-examples/usb/device/dev_lowlevel --------------+
 *   | Copyright (c) 2020 Raspberry Pi (Trading) Ltd.    |
 *   | SPDX-License-Identifier: BSD-3-Clause             |
 *   | URL: https://github.com/raspberrypi/pico-examples |
 *   +---------------------------------------------------+
 */

#ifndef _USB_VENDOR_SPECIFIC_H
#define _USB_VENDOR_SPECIFIC_H

#include "common.h"

static uint8_t data[] = {0,1,2,3,4,5,6};
static usb_endpoint_t *usb_ep_stat;
static usb_endpoint_t *usb_ep_cmd;
static usb_endpoint_t *usb_ep_data;

void usb_ep_cmd_handler(uint8_t *buf, uint16_t len) {
    uint32_t t = time_us_32();
    uint8_t t_arr[4];
    for (uint8_t i = 0; i < 4; i++) {
        t_arr[i] = (t >> (i * 8)) & 0xff;
    }
    usb_start_transfer(usb_ep_stat, (uint8_t*)t_arr, 4);
    printf("ep1<send time %08x\n", t);
}

void usb_ep_stat_handler(uint8_t *buf, uint16_t len) {
    usb_start_transfer(usb_ep_data, (uint8_t*)data, 7);
    printf("ep1< ep2 is sending data\n");
}

void usb_ep_data_handler(uint8_t* buf, uint16_t len) {
    usb_start_transfer(usb_ep_cmd, NULL, 1);
    printf("ep1< wait cmd\n");
}

static inline void usb_interface_init(void) {
    usb_ep_stat = &usb_eps[0];
    usb_ep_cmd = &usb_eps[1];
    usb_ep_data = &usb_eps[2];

    usb_ep_stat->handler = &usb_ep_stat_handler;
    usb_ep_cmd->handler = &usb_ep_cmd_handler;
    usb_ep_data->handler = &usb_ep_data_handler;

    usb_start_transfer(usb_ep_cmd, NULL, 1);

    printf("if_dat < wait for cmd...\n");
}

#endif