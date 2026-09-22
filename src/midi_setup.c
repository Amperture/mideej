#include "midi_setup.h"
#include "syscalls/device.h"
#include <zephyr/sys/printk.h>

struct midi_ump construct_packet(uint8_t type, uint8_t group, uint8_t command,
                                 uint8_t channel, uint8_t controller,
                                 uint8_t value) {
  struct midi_ump packet;
  packet.data[0] = (type << 28) | (group << 24) | (command << 20) |
                   (channel << 16) | (controller << 8) | (value);
  return packet;
}

struct device *midi_setup(void) {

  // We need to grab the USB device so we can send packets thru it.
  const struct device *mideej_midi = DEVICE_DT_GET(DT_NODELABEL(mideej_midi));
  while (device_is_ready(mideej_midi) == false) {
    printk("waiting for device to get ready\r\n");
  }
  return mideej_midi;
}
