/**
 * 
 *  Based on
 * 
 *  pico-examples/usb/device/dev_lowlevel -----------+
 *  | Copyright (c) 2020 Raspberry Pi (Trading) Ltd. |
 *  | SPDX-License-Identifier: BSD-3-Clause          |
 *  +------------------------------------------------+
 * 
 */

#include <string.h>

#include "usb_common.h"

/**
 * @brief Take a buffer pointer located in the USB RAM and return as an offset of the RAM.
 *
 * @param buf
 * @return uint32_t
 */
static inline uint32_t usb_buffer_offset(volatile uint8_t *buf) {
    return (uint32_t) buf ^ (uint32_t) usb_dpram;
}

/**
 * @brief Set up the endpoint control register for each endpoint.
 *
 */
void usb_set_endpoints(void) {
    // skip default endpoints(ep0_in & ep0_out) => start with i=2
    for (uint8_t i = 2; i < USB_ENDPOINTS_NUM; ++i) {
        uint32_t dpram_offset = usb_buffer_offset(endpoints[i]->data_buffer);
        uint32_t reg = EP_CTRL_ENABLE_BITS
                       | EP_CTRL_INTERRUPT_PER_BUFFER
                       | (endpoints[i]->transfer_type << EP_CTRL_BUFFER_TYPE_LSB)
                       | dpram_offset;
        *endpoints[i]->endpoint_control = reg;
    }
}

/**
 * @brief Set data on buffer of given endpoint & Access buffer-controll-register.
 *
 * @param ep the endpoint.
 * @param buf the data buffer to send. Only applicable if the endpoint is TX
 * @param len the length of the data in buf (this example limits max len to one packet - 64 bytes)
 */
void usb_start_transfer(struct usb_endpoint_t *ep, uint8_t *buf, uint16_t len) {
    // We are asserting that the length is <= 64 bytes for simplicity of the example.
    // For multi packet transfers see the tinyusb port.
    assert(len <= 64);

    // printf("Start transfer of len %d on ep addr 0x%x\n", len, ep->descriptor->bEndpointAddress);

    // Prepare buffer control register value
    uint32_t val = len | USB_BUF_CTRL_AVAIL;

    if (ep->is_direction_in) {
        // Need to copy the data from the user buffer to the usb memory
        memcpy((void *) ep->data_buffer, (void *) buf, len);
        // Mark as full
        val |= USB_BUF_CTRL_FULL;
    }

    // Set pid and flip for next transfer
    val |= ep->pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    if (ep->transfer_type != USB_TRANSFER_TYPE_ISOCHRONOUS) {
        ep->pid ^= 1u;
    }

    *ep->buffer_control = val;
}
