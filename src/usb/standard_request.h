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

#ifndef _USB_STANDARD_REQUEST_H
#define _USB_STANDARD_REQUEST_H

#include "send_descriptor.h"

// Global device address
static bool usb_should_set_addr = false;
static uint8_t usb_dev_addr = 0;
static volatile bool usb_configured = false;

/**
 *   ~~~~~~~~~~~~~
 *     bus reset
 *   ~~~~~~~~~~~~~
 */

static inline void usb_handle_bus_reset(void) {
    // Set address back to 0
    usb_dev_addr = 0;
    usb_should_set_addr = false;
    usb_hw->dev_addr_ctrl = 0;
    usb_configured = false;
    printf("+ bus reset\n");
}

/**
 *   ~~~~~~~~~~~~~~~~
 *     setup packet
 *   ~~~~~~~~~~~~~~~~
 */

/**
 * @brief Sends a zero length status packet back to the host.
 */
static inline void usb_send_ack(void) {
    usb_start_transfer(&usb_ep0_in, NULL, 0);
}

static inline void usb_receive_ack(void) {
    // Receive a zero length status packet from the host on EP0 OUT
    usb_start_transfer(&usb_ep0_out, NULL, 0);
}

/**
 * @brief Handles a SET_ADDR request from the host. The actual setting of the device address in
 * hardware is done in usb_ep0_in_handler. This is because we have to acknowledge the request first
 * as a device with address zero.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_set_address(volatile struct usb_setup_packet *pkt) {
    // Set address is a bit of a strange case because we have to send a 0 length status packet first with
    // address 0
    usb_dev_addr = (pkt->wValue & 0xff);
    printf("| + address=%d\n", usb_dev_addr);
    // Will set address in the callback phase
    usb_should_set_addr = true;
    usb_send_ack();
}

/**
 * @brief Handles a SET_CONFIGRUATION request from the host. Assumes one configuration so simply
 * sends a zero length status packet back to the host.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_set_configuration(volatile struct usb_setup_packet *pkt) {
    usb_configured = true;
    usb_send_ack();
    printf("| + set cfg\n");
}

/**
 * @brief Respond to a setup packet from the host.
 */
static inline void usb_handle_setup_packet(void) {
    volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet *) &usb_dpram->setup_packet;
    uint8_t req_direction = pkt->bmRequestType;
    uint8_t req = pkt->bRequest;

    printf("+ setup packet dir=%s req=%02x\n",
            req_direction==USB_DIR_IN ? "in" : "out",
            req
    );

    if (req_direction == USB_DIR_OUT) {
        switch (req) {
            case USB_REQUEST_SET_ADDRESS:
                usb_set_address(pkt);
                break;
            case USB_REQUEST_SET_CONFIGURATION:
                usb_set_configuration(pkt);
                break;
            default:
                break;
        }
        usb_send_ack();
    } else if (req_direction == USB_DIR_IN) {
        switch (req) {
            case USB_REQUEST_GET_DESCRIPTOR:
                usb_send_descriptor(pkt);
                break;
            default:
                break;
        }
    }
}

/**
 * @brief EP0 in transfer complete. Either finish the SET_ADDRESS process, or receive a zero
 * length status packet from the host.
 *
 * @param buf the data that was sent
 * @param len the length that was sent
 */
void usb_ep0_in_handler(uint8_t *buf, uint16_t len) {
    if (usb_should_set_addr) {
        // Set actual device address in hardware
        usb_hw->dev_addr_ctrl = usb_dev_addr;
        usb_should_set_addr = false;
        printf("| | set addr\n");
        usb_interface_init(); // start receive data
    } else {
        // handled setup packet (IN) => wait ACK from host
        usb_receive_ack();
    }
}

void usb_ep0_out_handler(uint8_t *buf, uint16_t len) {
    ;
}


#endif