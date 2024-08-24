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
#include "../sound.h"

#define USB_DATA_SIZE 60
static uint8_t usb_sending_data[USB_DATA_SIZE];
static uint16_t usb_sent_offset;

static usb_endpoint_t *usb_ep_stat;
static usb_endpoint_t *usb_ep_cmd;
static usb_endpoint_t *usb_ep_data;

void usb_ep_cmd_handler(uint8_t *buf, uint16_t len) {
    uint32_t st = start_time;

    usb_start_transfer(usb_ep_stat, (uint8_t*)&st, 4);
    printf("ep1<send time %08x\n", st);
}

void usb_ep_stat_handler(uint8_t *buf, uint16_t len) {
    // sem_sound_buf.core.
    memcpy(usb_sending_data, (uint8_t*)sound_buf, USB_DATA_SIZE);

    usb_sent_offset = 0;

    usb_start_transfer(usb_ep_data, usb_sending_data, USB_DATA_SIZE);
    printf("ep1< ep2 is sending data\n");
}

void usb_ep_data_handler(uint8_t* buf, uint16_t len) {
    if (usb_sent_offset < SOUND_BUF_SIZE) {
        uint16_t sending_len = MIN(SOUND_BUF_SIZE - usb_sent_offset, USB_DATA_SIZE);

        memcpy(usb_sending_data, (uint8_t*)sound_buf + usb_sent_offset, sending_len);

        usb_start_transfer(usb_ep_data, usb_sending_data, sending_len);
        usb_sent_offset += USB_DATA_SIZE;
    } else {
        usb_start_transfer(usb_ep_cmd, NULL, 1);
        printf("ep1< wait cmd\n");
    }
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