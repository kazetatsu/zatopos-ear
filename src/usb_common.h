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

#ifndef _USB_COMMON_H
#define _USB_COMMON_H

#include "usb_standard.h"

/**
 *   ~~~~~~~~~~~~~
 *     endpoints
 *   ~~~~~~~~~~~~~
 */

#define USB_ENDPOINTS_NUM 3

typedef void (*usb_ep_handler)(void);

// Struct in which we keep the endpoint configuration
struct usb_endpoint_t {
        // index of endpoint
        uint8_t id;
        bool is_direction_in;
        // 0/1:DATA0/1
        uint8_t pid;
        uint8_t transfer_type;

        // Pointers to endpoint + buffer control registers
        // in the USB controller DPSRAM
        volatile uint32_t *endpoint_control;
        volatile uint32_t *buffer_control;
        volatile uint8_t *data_buffer;

        // the function called when USB controller finish transfer
        usb_ep_handler handler;
};

#define EP0_IN_ADDR  (USB_DIR_IN  | 0) // Endpoint 0 , host <- device
#define EP0_OUT_ADDR (USB_DIR_OUT | 0) // Endpoint 0 , host -> device
#define EP1_IN_ADDR  (USB_DIR_IN  | 1) // Endpoint 1 , host <- device

// Global data buffer for EP0
static uint8_t ep0_buf[64];

// the functions called when USB controller finish transfer
void ep0_in_handler(void);
void ep0_out_handler(void);
void ep1_in_handler(void);

static struct usb_endpoint_t ep0_in = {
        .id = 0,
        .is_direction_in = true,
        .pid = 0,
        .transfer_type = USB_TRANSFER_TYPE_CONTROL,
        .endpoint_control = NULL, // NA for EP0
        .buffer_control = &usb_dpram->ep_buf_ctrl[0].in,
        .data_buffer = &usb_dpram->ep0_buf_a[0],
        .handler = &ep0_in_handler
};

static struct usb_endpoint_t ep0_out = {
        .id = 0,
        .is_direction_in = false,
        .pid = 0,
        .transfer_type = USB_TRANSFER_TYPE_CONTROL,
        .endpoint_control = NULL, // NA for EP0
        .buffer_control = &usb_dpram->ep_buf_ctrl[0].out,
        .data_buffer = &usb_dpram->ep0_buf_a[0],
        .handler = &ep0_out_handler
};

static struct usb_endpoint_t ep1_in = {
        .id = 0,
        .is_direction_in = true,
        .pid = 0,
        .transfer_type = USB_TRANSFER_TYPE_ISOCHRONOUS,
        .endpoint_control = &usb_dpram->ep_ctrl[0].in,
        .buffer_control = &usb_dpram->ep_buf_ctrl[1].in,
        // 1st free EPX buffer
        .data_buffer = &usb_dpram->epx_data[0 * 64],
        .handler = &ep1_in_handler
};

static struct usb_endpoint_t *endpoints[USB_ENDPOINTS_NUM] = {
        &ep0_in,
        &ep0_out,
        &ep1_in
};

/**
 *   ~~~~~~~~~~~~~~~
 *     descriptors
 *   ~~~~~~~~~~~~~~~
 */

// EP0 IN and OUT
static const struct usb_endpoint_descriptor ep0_out_descriptor = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP0_OUT_ADDR, // EP number 0, OUT from host (rx to device)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};

static const struct usb_endpoint_descriptor ep0_in_descriptor = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP0_IN_ADDR, // EP number 0, OUT from host (rx to device)
        .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
        .wMaxPacketSize   = 64,
        .bInterval        = 0
};

static const struct usb_endpoint_descriptor ep1_in_descriptor = {
        .bLength          = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType  = USB_DT_ENDPOINT,
        .bEndpointAddress = EP1_IN_ADDR,
        .bmAttributes     = USB_TRANSFER_TYPE_ISOCHRONOUS,
        .wMaxPacketSize   = 64,
        .bInterval        = 1  // transfer interval is 1 ms ( 2^(bInterval-1+3) * 125 = 1000[us] = 1[ms] )
};

static const struct usb_device_descriptor device_descriptor = {
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

static const struct usb_interface_descriptor interface_descriptor = {
        .bLength            = sizeof(struct usb_interface_descriptor),
        .bDescriptorType    = USB_DT_INTERFACE,
        .bInterfaceNumber   = 0,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 1,    // Interface has 1 endpoint
        .bInterfaceClass    = 0xff, // Vendor specific endpoint
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface         = 0
};

static const struct usb_configuration_descriptor config_descriptor = {
        .bLength         = sizeof(struct usb_configuration_descriptor),
        .bDescriptorType = USB_DT_CONFIG,
        .wTotalLength    = (sizeof(config_descriptor) +
                            sizeof(interface_descriptor) +
                            sizeof(ep1_in_descriptor)),
        .bNumInterfaces  = 1,
        .bConfigurationValue = 1, // Configuration 1
        .iConfiguration = 0,      // No string
        .bmAttributes = 0xc0,     // attributes: self powered, no remote wakeup
        .bMaxPower = 0x32         // 100ma
};

static const unsigned char lang_descriptor[] = {
        4,         // bLength
        0x03,      // bDescriptorType == String Descriptor
        0x09, 0x04 // language id = us english
};

static const unsigned char *descriptor_strings[] = {
        (unsigned char *) "kazetatsu",  // Vendor
        (unsigned char *) "zatopos"     // Product
};

static volatile bool configured = false;

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

/**
 *   ~~~~~~~~~~~~~
 *     functions
 *   ~~~~~~~~~~~~~
 */

/**
 * @brief Set up the endpoint control register for each endpoint.
 *
 */
void usb_set_endpoints(void);

/**
 * @brief Set data on buffer of given endpoint & Access buffer-controll-register.
 *
 * @param ep the endpoint.
 * @param buf the data buffer to send. Only applicable if the endpoint is TX
 * @param len the length of the data in buf (this example limits max len to one packet - 64 bytes)
 */
void usb_start_transfer(struct usb_endpoint_t *ep, uint8_t *buf, uint16_t len);

#endif