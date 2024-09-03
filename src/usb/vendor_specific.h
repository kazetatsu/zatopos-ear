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
#include "../consts.h"

#define USB_DATA_SIZE 60

static uint16_t *usb_sending_sound;
static uint16_t usb_sending_checker;
static uint16_t usb_sent_offset;

static bool usb_need_send = false;

static uint8_t usb_stat[USB_DATA_SIZE];

static usb_endpoint_t *usb_ep_cmd;
static usb_endpoint_t *usb_ep_data;

void usb_ep_cmd_handler(uint8_t *buf, uint16_t len) {
    if (len == 1) {
        switch (buf[0]) {
            case CMND_SEND_SOUND:
                if (usb_sending_checker != sound_checker) {
                    usb_sending_sound = sound_bufs[sound_front];
                    usb_sending_checker = sound_checker;

                    usb_need_send = true;
                    usb_start_transfer(usb_ep_data, (uint8_t*)usb_sending_sound, USB_DATA_SIZE);
                    usb_sent_offset = USB_DATA_SIZE;
                } else {
                    usb_stat[0] = 0xff;
                    usb_stat[1] = STAT_NO_NEW_DATA;

                    usb_need_send = false;
                    usb_start_transfer(usb_ep_data, usb_stat, USB_DATA_SIZE);
                }
                break;

            default:
                break;
        }
    }
    usb_start_transfer(usb_ep_cmd, NULL, 1);
}

void usb_ep_data_handler(uint8_t* buf, uint16_t len) {
    if (usb_need_send && usb_sent_offset < SOUND_BUF_SIZE) {
        uint16_t sending_len = MIN(SOUND_BUF_SIZE - usb_sent_offset, USB_DATA_SIZE);

        usb_start_transfer(usb_ep_data, (uint8_t*)usb_sending_sound + usb_sent_offset, sending_len);
        usb_sent_offset += USB_DATA_SIZE;
    }
}

static inline void usb_interface_init(void) {
    usb_ep_data = &usb_eps[0];
    usb_ep_cmd = &usb_eps[1];

    usb_ep_data->handler = &usb_ep_data_handler;
    usb_ep_cmd->handler = &usb_ep_cmd_handler;

    memset(usb_stat, 0, USB_DATA_SIZE);

    usb_start_transfer(usb_ep_cmd, NULL, 1);

    printf("if_dat < wait for cmd...\n");
}

#endif