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

#ifndef _USB_SETUP_H
#define _USB_SETUP_H

void usb_bus_reset(void);
void usb_handle_setup_packet(void);

#endif