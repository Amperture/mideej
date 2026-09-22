#include "usb_setup.h"
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usbd.h>

/* This line creates a usbd_context struct, no enumeration is happening as of
 * yet.
 *
 * `mideej_ctx` is the struct's name which will be used later to
 *   reference
 *
 * the second input is the actual binding, zephyr_udc0 is the devicetree label
 *   for usb_otg, note that we are getting the nodelabel from devicetree
 *
 * the last two are the VID and PID
 */
// TODO: Get my own VID and PID from pid.codes
USBD_DEVICE_DEFINE(mideej_ctx, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)), 0x2fe3,
                   0x0001);

/* we're still not enumerating devices yet, but these lines define a few things
 * for the device.
 *
 * These are ways for the device to "describe" itself to the host machine
 *
 * the first line defines the language the device speaks, as in literal
 * human-spoken language, at the time of writing, the macro has no options
 * as the only language supported is US English
 *
 * the second line defines the manufacturer, which we're making up
 *
 * the third line defines the product name, which we're also making up
 *
 * the fourth line will be used later, and is describing the configuration
 * to the host
 */
USBD_DESC_LANG_DEFINE(mideej_desc_lang);
USBD_DESC_MANUFACTURER_DEFINE(mideej_desc_mfr, "Amperture Enginerding");
USBD_DESC_PRODUCT_DEFINE(mideej_desc_product, "mideej");
USBD_DESC_CONFIG_DEFINE(mideej_desc_cfg, "Full-Speed Configuration");

/* this line sets up some configuration details
 *
 * First input is just the usbd_context struct's name
 *
 * The second input is a bitmask of attributes, for the time being we're lying
 * and saying the board is powering itself thru perhaps a battery, I assume
 * we'll learn about which attributes we have an option for later.
 *
 * The third input is our requested power draw from the USB host. Assume
 * that you are requesting 2 miliamps times the number you're inputting, so
 * to get 10mA you want to put in 5.
 *
 */
// USBD_CONFIGURATION_DEFINE(mideej_cfg, USB_SCD_SELF_POWERED, 100,
// &mideej_desc_cfg);
USBD_CONFIGURATION_DEFINE(mideej_cfg, 0, 100, &mideej_desc_cfg);

uint8_t usb_setup(void) {
  /* Here is where we actually start instantiating a device
   * we use the config and descriptors defined above and construct our USB
   * device, but we still haven't spoken to the host yet.
   *
   * Order matters: langauge, manufacturer, product
   */
  usbd_add_descriptor(&mideej_ctx, &mideej_desc_lang);
  usbd_add_descriptor(&mideej_ctx, &mideej_desc_mfr);
  usbd_add_descriptor(&mideej_ctx, &mideej_desc_product);
  // usbd_add_descriptor(&mideej_ctx, &mideej_desc_cfg);

  /* Now we add the configuration to the context.
   * Still not speaking to the host.
   */
  if (usbd_add_configuration(&mideej_ctx, USBD_SPEED_FS, &mideej_cfg) != 0) {
    printk("Failed to add configuration.\r\n");
    return 1;
  }

  /* Now we add the device class info
   */
  if (usbd_register_class(&mideej_ctx, "midi_0", USBD_SPEED_FS, 1) != 0) {
    printk("Failed to register USB class.\r\n");
    return 1;
  }

  /* set up the Class, Subclass, and Protocol
   * for our case this is "Miscellaneous", 0x02 is "Common Class", 0x01 is "IAD"
   */
  /*
  usbd_device_set_code_triple(&mideej_ctx, USBD_SPEED_FS, USB_BCC_MISCELLANEOUS,
                              0x02, 0x01);
  */

  /* USB MIDI Descriptor Codes
   * Base Class (bInterfaceClass): 0x01 (Audio Device Class)
   * SubClass (bInterfaceSubClass): 0x03 (MIDI Streaming)
   * Protocol (bInterfaceProtocol): 0x00 (Standard/Unspecified Audio Protocol)
   */
  usbd_device_set_code_triple(&mideej_ctx, USBD_SPEED_FS, USB_BCC_MISCELLANEOUS,
                              0x02, 0x01);

  // Final conglomeration of the configurations
  if (usbd_init(&mideej_ctx) != 0) {
    printk("USB INIT FAIL\r\n");
    return 1;
  }

  // Now let's talk to the host machine and tell it we're a USB device to
  // instantiate
  if (usbd_enable(&mideej_ctx) != 0) {
    printk("USB ENABLE FAIL\r\n");
    return 1;
  }

  // if everything goes well, we can return 0 and be confident that things
  // have worked
  return 0;
}
