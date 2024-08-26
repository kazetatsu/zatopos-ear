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
static uint16_t usb_sending_checker;
static uint16_t usb_sent_offset;

static uint8_t usb_stat[USB_DATA_SIZE];

static usb_endpoint_t *usb_ep_stat;
static usb_endpoint_t *usb_ep_cmd;
static usb_endpoint_t *usb_ep_data;

void usb_ep_cmd_handler(uint8_t *buf, uint16_t len) {
    uint32_t st = start_time;
    memcpy(usb_stat + 1, &st, 4);
    usb_stat[0] = 0x10u;

    usb_sending_checker = sound_checker;
    usb_sent_offset = 0;

    usb_start_transfer(usb_ep_data, (uint8_t*)usb_stat, 5);
}

void usb_ep_data_handler(uint8_t* buf, uint16_t len) {
    if (usb_sent_offset < SOUND_BUF_SIZE) {
        uint16_t sending_len = MIN(SOUND_BUF_SIZE - usb_sent_offset, USB_DATA_SIZE);

        if (usb_sending_checker != sound_checker) {
            /**ここに送信しきれなかった旨表すステータスを返す処理 */
        }
        usb_start_transfer(usb_ep_data, (uint8_t*)sound_buf + usb_sent_offset, sending_len);
        usb_sent_offset += USB_DATA_SIZE;
    } else {
        usb_start_transfer(usb_ep_cmd, NULL, 1);
        printf("ep1< wait cmd\n");
    }
}

static inline void usb_interface_init(void) {
    usb_ep_data = &usb_eps[0];
    usb_ep_cmd = &usb_eps[1];

    usb_ep_data->handler = &usb_ep_data_handler;
    usb_ep_cmd->handler = &usb_ep_cmd_handler;

    usb_start_transfer(usb_ep_cmd, NULL, 1);

    printf("if_dat < wait for cmd...\n");
}

#endif