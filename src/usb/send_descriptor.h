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

#ifndef _USB_SEND_DESC_H
#define _USB_SEND_DESC_H

#include "common.h"

// Global data buffer for EP0
static uint8_t usb_ep0_buf[64];

/**
 * @brief Send device descriptor to host
 */
static inline void usb_send_device_descriptor(uint16_t requested_len) {
    // Always respond with pid 1
    (&usb_ep0_in)->pid = 1;
    usb_start_transfer(&usb_ep0_in, (uint8_t *)&usb_dev_desc, MIN(sizeof(struct usb_device_descriptor), requested_len));
    printf("| | sending dev desc\n");
}

/**
 * @brief Send the configuration descriptor (and potentially the configuration and endpoint descriptors) to the host.
 *
 * @param pkt the setup packet received from the host.
 */
void usb_send_config_descriptor(uint16_t requested_len) {
    uint8_t *buf = &usb_ep0_buf[0];

    // First request will want just the config descriptor
    memcpy((void *) buf, &usb_cfg_desc, sizeof(struct usb_configuration_descriptor));
    buf += sizeof(struct usb_configuration_descriptor);

    // If we more than just the config descriptor copy it all
    if (requested_len >= (&usb_cfg_desc)->wTotalLength) {
        memcpy((void *) buf, &usb_if_desc, sizeof(struct usb_interface_descriptor));
        buf += sizeof(struct usb_interface_descriptor);

        for (uint8_t i = 0; i < USB_NUM_ENABLED_ENDPOINTS; i++) {
            // Copy all the endpoint descriptors starting from EP1
            memcpy((void *) buf, &usb_ep_descs[i], sizeof(struct usb_endpoint_descriptor));
            buf += sizeof(struct usb_endpoint_descriptor);
        }
    }

    // Send data
    // Get len by working out end of buffer subtract start of buffer
    uint32_t len = (uint32_t) buf - (uint32_t) &usb_ep0_buf[0];
    (&usb_ep0_in)->pid = 1;
    usb_start_transfer(&usb_ep0_in, &usb_ep0_buf[0], MIN(len, requested_len));

    printf("| | sending cfg desc\n");
}

/**
 * @brief Send the requested string descriptor to the host.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_send_string_descriptor(uint8_t id) {
    // Lower 8bits
    // 0 : Send language id
    // 1 : Send vendor name
    // 2 : Send product name
    uint16_t len;

    if (id == 0) {
        len = 4;
        memcpy(&usb_ep0_buf[0], usb_string_desc_lang, len);
    } else {
        const unsigned char* str = usb_strings[id - 1];

        // 2 for bLength + bDescriptorType + strlen * 2 because string is unicode. i.e. other byte will be 0
        uint8_t bLength = 2 + (strlen((const char *)str) * 2);
        static const uint8_t bDescriptorType = 0x03;

        volatile uint8_t *buf = &usb_ep0_buf[0];
        *buf++ = bLength;
        *buf++ = USB_DT_STRING;

        uint8_t c;

        do {
            c = *str++;
            *buf++ = c;
            *buf++ = 0;
        } while (c != '\0');

        len = bLength;
    }

    (&usb_ep0_in)->pid = 1;
    usb_start_transfer(&usb_ep0_in, &usb_ep0_buf[0], len);
    printf("| | sending %dth str desc\n", id);
}

/**
 * @brief Handles a GET_DESCRIPTOR request from the host.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_send_descriptor(volatile struct usb_setup_packet *pkt) {
    // Upper 8 bits : descriptor type
    // Lower 8 bits : index of descriptor
    uint16_t wValue = pkt->wValue;
    uint16_t wLength = pkt->wLength;
    uint8_t descriptor_type = wValue >> 8;
    printf("| + desc %04x\n", descriptor_type);

    switch (descriptor_type) {
        case USB_DT_DEVICE:
            usb_send_device_descriptor(wLength);
            break;
        case USB_DT_CONFIG:
            usb_send_config_descriptor(wLength);
            break;
        case USB_DT_STRING:
            usb_send_string_descriptor(wValue & 0xff);
            break;
        default:
            break;
    }
}

#endif