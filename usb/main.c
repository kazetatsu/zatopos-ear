#include "dev_lowlevel_ctr.h"

void ep0_in_handler(uint8_t *buf, uint16_t len) {
    if (should_set_address) {
        // Set actual device address in hardware
        usb_hw->dev_addr_ctrl = dev_addr;
        should_set_address = false;
    } else {
        // Receive a zero length status packet from the host on EP0 OUT
        struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP0_OUT_ADDR);
        usb_start_transfer(ep, NULL, 0);
    }
}

void ep0_out_handler(uint8_t *buf, uint16_t len) {
    ;
}

// Device specific functions
void ep1_out_handler(uint8_t *buf, uint16_t len) {
    printf("RX %d bytes from host\n", len);
    // Send data back to host
    // struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP2_IN_ADDR);
    // usb_start_transfer(ep, buf, len);
}

void ep2_in_handler(uint8_t *buf, uint16_t len) {
    printf("Sent %d bytes to host\n", len);
    // Get ready to rx again from host
    usb_start_transfer(usb_get_endpoint_configuration(EP1_OUT_ADDR), NULL, 64);
}

int main() {
    stdio_init_all();
    printf("USB Device Low-Level hardware example\n");
    usb_device_init();

    // Wait until configured
    while (!configured) {
        tight_loop_contents();
    }

    // Get ready to rx from host
    usb_start_transfer(usb_get_endpoint_configuration(EP1_OUT_ADDR), NULL, 64);

    uint8_t buf[] = {0xca, 0xfe};

    // Everything is interrupt driven so just loop here
    while (1) {
        sleep_ms(100);
        usb_start_transfer(usb_get_endpoint_configuration(EP2_IN_ADDR), buf, 2);
    }

    return 0;
}
