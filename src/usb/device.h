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

#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

#include <hardware/resets.h>
#include <hardware/irq.h>

#include "vendor_specific.h"
#include "endpoint.h"
#include "standard_request.h"

void usb_handle_root(void) {
    uint32_t status = usb_hw->ints;
    uint32_t handled = 0u;

    // Buffer status, one or more buffers have completed
    if (status & USB_INTS_BUFF_STATUS_BITS) {
        usb_handle_buff_status();
        handled |= USB_INTS_BUFF_STATUS_BITS;
    }

    if (status & USB_INTS_SETUP_REQ_BITS) {
        usb_hw_clear->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        usb_handle_setup_packet();
        handled |= USB_INTS_SETUP_REQ_BITS;
    }

    if (status &  USB_INTS_BUS_RESET_BITS) {
        usb_hw_clear->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        usb_handle_bus_reset();
        handled |= USB_INTS_BUS_RESET_BITS;
    }
}

void usb_device_init(void) {
    // Reset usb controller
    reset_block(RESETS_RESET_USBCTRL_BITS);
    unreset_block_wait(RESETS_RESET_USBCTRL_BITS);

    // Clear any previous state in dpram just in case
    memset(usb_dpram, 0, sizeof(*usb_dpram)); // <1>

    // Mux the controller to the onboard usb phy
    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;

    // Force VBUS detect so the device thinks it is plugged into a host
    usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;

    // Enable the USB controller in device mode.
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    // Enable an interrupt per EP0 transaction
    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS; // <2>

    // Enable interrupts for when a buffer is done, when the bus is reset,
    // and when a setup packet is received
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS |
                   USB_INTS_BUS_RESET_BITS |
                   USB_INTS_SETUP_REQ_BITS;

    // Present full speed device by enabling pull up on DP
    usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;

    // Enable USB interrupt at processor
    irq_set_priority(USBCTRL_IRQ, PICO_DEFAULT_IRQ_PRIORITY);
    irq_set_exclusive_handler(USBCTRL_IRQ, &usb_handle_root);
    irq_set_enabled(USBCTRL_IRQ, true);

    usb_endpoints_init();

    printf("initialize done\n");
}

#endif