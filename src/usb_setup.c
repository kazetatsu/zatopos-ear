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

// USB register definitions from pico-sdk
#include "hardware/regs/usb.h"
// USB hardware struct definitions from pico-sdk
#include "hardware/structs/usb.h"

#include "usb_common.h"
#include "usb_setup.h"

// Global device address
static bool should_set_address = false;
static uint8_t dev_addr = 0;

/**
 * @brief Handle a BUS RESET from the host by setting the device address back to 0.
 *
 */
void usb_bus_reset(void) {
    // Set address back to 0
    dev_addr = 0;
    should_set_address = false;
    usb_hw->dev_addr_ctrl = 0;
    configured = false;
}

/**
 * @brief Sends a zero length status packet back to the host.
 */
static inline void usb_acknowledge_out_request(void) {
    usb_start_transfer(&ep0_in, NULL, 0);
}

/**
 * @brief Send device descriptor to host
 *
 */
static inline void usb_handle_device_descriptor(volatile struct usb_setup_packet *pkt) {
    const struct usb_device_descriptor *d = &device_descriptor;
    // Always respond with pid=DATA1
    ep0_in.pid = 1;
    usb_start_transfer(&ep0_in, (uint8_t *) d, MIN(sizeof(struct usb_device_descriptor), pkt->wLength));
}

/**
 * @brief Send the configuration descriptor (and potentially the configuration and endpoint descriptors) to the host.
 *
 * @param pkt the setup packet received from the host.
 */
static inline void usb_handle_config_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t *buf = &ep0_buf[0];

    // First request will want just the config descriptor
    const struct usb_configuration_descriptor *d = &config_descriptor;
    memcpy((void *) buf, d, sizeof(struct usb_configuration_descriptor));
    buf += sizeof(struct usb_configuration_descriptor);

    // If we more than just the config descriptor copy it all
    if (pkt->wLength >= d->wTotalLength) {
        memcpy((void *) buf, &interface_descriptor, sizeof(struct usb_interface_descriptor));
        buf += sizeof(struct usb_interface_descriptor);

        // Copy all the endpoint descriptors starting from EP1
        memcpy((void *) buf, &ep1_in_descriptor, sizeof(struct usb_endpoint_descriptor));
        buf += sizeof(struct usb_endpoint_descriptor);
    }

    // Send data
    // Get len by working out end of buffer subtract start of buffer
    uint32_t len = (uint32_t) buf - (uint32_t) &ep0_buf[0];
    usb_start_transfer(&ep0_in, &ep0_buf[0], MIN(len, pkt->wLength));
}

/**
 * @brief Given a C string, fill the EP0 data buf with a USB string descriptor for that string.
 *
 * @param C string you would like to send to the USB host
 * @return the length of the string descriptor in EP0 buf
 */
static inline uint8_t usb_prepare_string_descriptor(const unsigned char *str) {
    // 2 for bLength + bDescriptorType + strlen * 2 because string is unicode. i.e. other byte will be 0
    uint8_t bLength = 2 + (strlen((const char *)str) * 2);
    static const uint8_t bDescriptorType = 0x03;

    volatile uint8_t *buf = &ep0_buf[0];
    *buf++ = bLength;
    *buf++ = bDescriptorType;

    uint8_t c;

    do {
        c = *str++;
        *buf++ = c;
        *buf++ = 0;
    } while (c != '\0');

    return bLength;
}

/**
 * @brief Send the requested string descriptor to the host.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_handle_string_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t i = pkt->wValue & 0xff;
    uint8_t len = 0;

    if (i == 0) {
        len = 4;
        memcpy(&ep0_buf[0], &lang_descriptor, len);
    } else {
        // Prepare fills in ep0_buf
        len = usb_prepare_string_descriptor(descriptor_strings[i - 1]);
    }

    usb_start_transfer(&ep0_in, &ep0_buf[0], MIN(len, pkt->wLength));
}

/**
 * @brief Handles a SET_ADDR request from the host. The actual setting of the device address in
 * hardware is done in ep0_in_handler. This is because we have to acknowledge the request first
 * as a device with address zero.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_set_device_address(volatile struct usb_setup_packet *pkt) {
    // Set address is a bit of a strange case because we have to send a 0 length status packet first with
    // address 0
    dev_addr = (pkt->wValue & 0xff);
    // printf("Set address %d\r\n", dev_addr);
    // Will set address in the callback phase
    should_set_address = true;
    usb_acknowledge_out_request();
}

/**
 * @brief Handles a SET_CONFIGRUATION request from the host. Assumes one configuration so simply
 * sends a zero length status packet back to the host.
 *
 * @param pkt the setup packet from the host.
 */
static inline void usb_set_device_configuration(volatile struct usb_setup_packet *pkt) {
    // Only one configuration so just acknowledge the request
    // printf("Device Enumerated\r\n");
    usb_acknowledge_out_request();
    configured = true;
}

/**
 * @brief Respond to a setup packet from the host.
 *
 */
void usb_handle_setup_packet(void) {
    volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet *) &usb_dpram->setup_packet;
    uint8_t req_direction = pkt->bmRequestType;
    uint8_t req = pkt->bRequest;

    // // Reset PID to 1 for EP0 IN
    // usb_get_endpoint_configuration(EP0_IN_ADDR)->next_pid = 1u;

    if (req_direction == USB_DIR_OUT) {
        if (req == USB_REQUEST_SET_ADDRESS) {
            usb_set_device_address(pkt);
        } else if (req == USB_REQUEST_SET_CONFIGURATION) {
            usb_set_device_configuration(pkt);
        } else {
            usb_acknowledge_out_request();
            // printf("Other OUT request (0x%x)\r\n", pkt->bRequest);
        }
    } else if (req_direction == USB_DIR_IN) {
        if (req == USB_REQUEST_GET_DESCRIPTOR) {
            uint16_t descriptor_type = pkt->wValue >> 8;

            switch (descriptor_type) {
                case USB_DT_DEVICE:
                    usb_handle_device_descriptor(pkt);
                    // printf("GET DEVICE DESCRIPTOR\r\n");
                    break;

                case USB_DT_CONFIG:
                    usb_handle_config_descriptor(pkt);
                    // printf("GET CONFIG DESCRIPTOR\r\n");
                    break;

                case USB_DT_STRING:
                    usb_handle_string_descriptor(pkt);
                    // printf("GET STRING DESCRIPTOR\r\n");
                    break;

                default:
                    // printf("Unhandled GET_DESCRIPTOR type 0x%x\r\n", descriptor_type);
                    ;
            }
        } else {
            // printf("Other IN request (0x%x)\r\n", pkt->bRequest);
            ;
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
void ep0_in_handler(void) {
    if (should_set_address) {
        // Set actual device address in hardware
        usb_hw->dev_addr_ctrl = dev_addr;
        should_set_address = false;
    } else {
        // Receive a zero length status packet from the host on EP0 OUT
        usb_start_transfer(&ep0_out, NULL, 0);
    }
}

/**
 * @brief EP0 out transfer complete. Do nothing 
 *
 * @param buf the data that was received
 * @param len the length that was received
 */
void ep0_out_handler(void) {
    ;
}
