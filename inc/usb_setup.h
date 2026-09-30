#ifndef USB_SETUP_H
#define USB_SETUP_H

#include <stdint.h>
#include <zephyr/kernel.h>

// Public Functions
uint8_t usb_setup(void);

extern struct k_mutex usb_mutex;

#endif // USB_SETUP_H
