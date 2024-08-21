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

#ifndef _USB_ENDPOINT_H
#define _USB_ENDPOINT_H

#include "common.h"

static inline void usb_check_and_call_ep_handler(usb_endpoint_t *ep, uint32_t buf_stat) {
    // IN transfer for even, OUT transfer for odd
    uint32_t bit = 1u << (2 * ep->id);
    if (!ep->is_direction_in) {
        bit <<= 1;
    }

    if (buf_stat & bit) {
        // clear this in advance
        usb_hw_clear->buf_status = bit;
        uint16_t len = *ep->buf_ctrl & USB_BUF_CTRL_LEN_MASK;
        uint8_t *buf = (uint8_t*)ep->data_buf;
        printf("| + ep%d_%s %d bytes data\n",
                ep->id,
                ep->is_direction_in ? "in sent" : "out received",
                len
        );
        ep->handler(buf, len);
    }
}

/**
 * @brief Handle a "buffer status" irq. This means that one or more
 * buffers have been sent / received. Notify each endpoint where this
 * is the case.
 */
static inline void usb_handle_buff_status() {
    uint32_t buffers = usb_hw->buf_status;

    printf("+ buf stat %08x\n", buffers);

    usb_check_and_call_ep_handler(&usb_ep0_in, buffers);
    usb_check_and_call_ep_handler(&usb_ep0_out, buffers);

    for (uint8_t i = 0; i < USB_NUM_ENABLED_ENDPOINTS; i++) {
        usb_check_and_call_ep_handler(&usb_eps[i], buffers);
    }
}

#endif