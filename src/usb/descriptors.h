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

#ifndef _USB_DESCRIPTORS_H
#define _USB_DESCRIPTORS_H

#include "structs.h"

static const unsigned char usb_string_desc_lang[] = {
        4,         // bLength
        0x03,      // bDescriptorType == String Descriptor
        0x09, 0x04 // language id = us english
};

static const unsigned char *usb_strings[] = {
        (unsigned char *) "kazetatsu",    // Vendor
        (unsigned char *) "usbdev" // Product
};

static const struct usb_device_descriptor usb_dev_desc = {
        .bLength         = sizeof(struct usb_device_descriptor),
        .bDescriptorType = USB_DT_DEVICE,
        .bcdUSB          = 0x0110, // USB 1.1 device
        .bDeviceClass    = 0,      // Specified in interface descriptor
        .bDeviceSubClass = 0,      // No subclass
        .bDeviceProtocol = 0,      // No protocol
        .bMaxPacketSize0 = 64,     // Max packet size for ep0
        .idVendor        = 0x0000, // Your vendor id
        .idProduct       = 0x0001, // Your product ID
        .bcdDevice       = 0,      // No device revision number
        .iManufacturer   = 1,      // Manufacturer string index
        .iProduct        = 2,      // Product string index
        .iSerialNumber = 0,        // No serial number
        .bNumConfigurations = 1    // One configuration
};

#define USB_NUM_ENABLED_ENDPOINTS 3

static const struct usb_endpoint_descriptor usb_ep_descs[USB_NUM_ENABLED_ENDPOINTS] = {
        {
                .bLength          = sizeof(struct usb_endpoint_descriptor),
                .bDescriptorType  = USB_DT_ENDPOINT,
                .bEndpointAddress = USB_DIR_IN | 1, // EP number 1, IN(device->host)
                .bmAttributes     = USB_TRANSFER_TYPE_BULK,
                .wMaxPacketSize   = 64,
                .bInterval        = 0
        },
        {
                .bLength          = sizeof(struct usb_endpoint_descriptor),
                .bDescriptorType  = USB_DT_ENDPOINT,
                .bEndpointAddress = USB_DIR_OUT | 1, // EP number 1, OUT(host->device)
                .bmAttributes     = USB_TRANSFER_TYPE_BULK,
                .wMaxPacketSize   = 64,
                .bInterval        = 0
        },
        {
                .bLength          = sizeof(struct usb_endpoint_descriptor),
                .bDescriptorType  = USB_DT_ENDPOINT,
                .bEndpointAddress = USB_DIR_IN | 2, // EP number 2, IN(device->host)
                .bmAttributes     = USB_TRANSFER_TYPE_BULK,
                .wMaxPacketSize   = 64,
                .bInterval        = 0
        }
};

static const struct usb_interface_descriptor usb_if_desc = {
        .bLength            = sizeof(struct usb_interface_descriptor),
        .bDescriptorType    = USB_DT_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 3,    // Interface has 3 endpoints exclude ep0
        .bInterfaceClass    = 0xff, // Vendor specific endpoint
        .bInterfaceSubClass = 0xff,
        .bInterfaceProtocol = 0xff,
        .iInterface         = 0
};

static const struct usb_configuration_descriptor usb_cfg_desc = {
        .bLength         = sizeof(struct usb_configuration_descriptor),
        .bDescriptorType = USB_DT_CONFIG,
        .wTotalLength    = (sizeof(usb_cfg_desc) +
                            sizeof(usb_if_desc)  +
                            sizeof(usb_ep_descs)),
        .bNumInterfaces  = 1,
        .bConfigurationValue = 1, // Configuration 1
        .iConfiguration = 0,      // No string
        .bmAttributes = 0xc0,     // attributes: self powered, no remote wakeup
        .bMaxPower = 0x32         // 100mA
};

#endif