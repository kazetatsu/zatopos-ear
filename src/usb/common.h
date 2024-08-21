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

#ifndef _USB_COMMON_H
#define _USB_COMMON_H

#include <stdio.h>
#include <string.h> // for memcpy
#include <pico/stdlib.h>

#include "descriptors.h"

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

void usb_ep0_in_handler(uint8_t* buf, uint16_t len);
void usb_ep0_out_handler(uint8_t* buf, uint16_t len);

static usb_endpoint_t usb_ep0_in = {
    .id = 0,
    .is_direction_in = true,
    .transfer_type = USB_TRANSFER_TYPE_CONTROL,
    .handler = &usb_ep0_in_handler,
    .buf_ctrl = &usb_dpram->ep_buf_ctrl[0].in,
    .data_buf = &usb_dpram->ep0_buf_a[0],
    .pid = 0
};

static usb_endpoint_t usb_ep0_out = {
    .id = 0,
    .is_direction_in = false,
    .transfer_type = USB_TRANSFER_TYPE_CONTROL,
    .handler = &usb_ep0_out_handler,
    .buf_ctrl = &usb_dpram->ep_buf_ctrl[0].out,
    .data_buf = &usb_dpram->ep0_buf_a[0],
    .pid = 0
};

static inline void usb_interface_init(void);

static usb_endpoint_t usb_eps[USB_NUM_ENABLED_ENDPOINTS];

static inline void usb_endpoints_init(void) {
    uint8_t *data_buf = usb_dpram->epx_data;

    for (uint8_t i = 0; i < USB_NUM_ENABLED_ENDPOINTS; ++i) {
        usb_endpoint_t *ep = &usb_eps[i];
        const struct usb_endpoint_descriptor *desc = &usb_ep_descs[i];

        // Assign initial values to ep's members
        ep->transfer_type = desc->bmAttributes & 0x03;
        ep->id = desc->bEndpointAddress & 0x0f;
        ep->is_direction_in = (desc->bEndpointAddress & 0xf0) == USB_DIR_IN;
        ep->handler = NULL;
        if (ep->is_direction_in) {
            ep->buf_ctrl = &usb_dpram->ep_buf_ctrl[ep->id].in;
        } else {
            ep->buf_ctrl = &usb_dpram->ep_buf_ctrl[ep->id].out;
        }
        ep->data_buf = data_buf;
        ep->pid = 0;

        // Write endpoint controll register
        uint32_t offset = (uint32_t)ep->data_buf ^ (uint32_t)usb_dpram;
        uint32_t val = EP_CTRL_ENABLE_BITS
                     | EP_CTRL_INTERRUPT_PER_BUFFER
                     | (ep->transfer_type << EP_CTRL_BUFFER_TYPE_LSB)
                     | offset;

        if (ep->is_direction_in) {
            usb_dpram->ep_ctrl[ep->id - 1].in = val;
        } else {
            usb_dpram->ep_ctrl[ep->id - 1].out = val;
        }

        // Put the buffer address forward for next enendpoint
        data_buf += desc->wMaxPacketSize;
    }
}

/**
 * @brief Starts a transfer on a given endpoint.
 *
 * @param ep the endpoint configuration.
 * @param buf the data buffer to send. Only applicable if the endpoint is TX
 * @param len the length of the data in buf (this example limits max len to one packet - 64 bytes)
 */
void usb_start_transfer(usb_endpoint_t *ep, uint8_t *buf, uint16_t len) {
    // We are asserting that the length is <= 64 bytes for simplicity of the example.
    // For multi packet transfers see the tinyusb port.
    assert(len <= 64);

    // Prepare buffer control register value
    uint32_t val = len | USB_BUF_CTRL_AVAIL;

    if (ep->is_direction_in) {
        // Need to copy the data from the user buffer to the usb memory
        memcpy((void *) ep->data_buf, (void *) buf, len);
        // Mark as full
        val |= USB_BUF_CTRL_FULL;
    }

    // Set pid and flip for next transfer
    val |= ep->pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    if (ep->transfer_type != USB_TRANSFER_TYPE_ISOCHRONOUS) {
        ep->pid ^= 1u;
    }

    *ep->buf_ctrl = val;
}

#endif