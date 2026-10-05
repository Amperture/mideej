#include "midi_setup.h"
#include "config.h"
#include "syscalls/device.h"
#include "usb_setup.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct midi_ump construct_packet(uint8_t type, uint8_t group, uint8_t command,
                                 uint8_t channel, uint8_t controller,
                                 uint8_t value) {
  struct midi_ump packet;
  packet.data[0] = (type << 28) | (group << 24) | (command << 20) |
                   (channel << 16) | (controller << 8) | (value);
  return packet;
}

const struct device *midi_setup(void) {

  // We need to grab the USB device so we can send packets thru it.
  const struct device *mideej_midi = DEVICE_DT_GET(DT_NODELABEL(mideej_midi));
  while (device_is_ready(mideej_midi) == false) {
    printk("waiting for MIDI device to get ready\r\n");
  }
  return mideej_midi;
}

// The semaphore is also extremely overengineered here deliberately. It is not
// strictly needed, but used as an example of how a semaphore might be used
// to process new MIDI signals being ready to go out as sending out a MIDI CC
// signal is an "event".
// K_SEM_DEFINE(midi_send_sem, 0, 32);

// In reality the Message Queue is really all that's necessary here, and in fact
// the message queue acts as a semaphore with a data payload. Again, deliberate
// overengineering as a learning experience.
K_MSGQ_DEFINE(midi_send_msgq, sizeof(struct midi_send_value), 64, 1);

void midi_send_thread(void *p1, void *p2, void *p3) {
  printk("MIDI thread initializing\r\n");
  struct midi_send_value midi_rx_packet;

  // Initialize the USB Device here, since there's not another task to take
  // care of it. This will likely be moved or taken care of elsewhere if a
  // better init process is thought of.
  if (usb_setup() != 0) {
    printk("Something went wrong with USB setup!\r\n");
  }

  const struct device *mideej_midi = midi_setup();
  printk("MIDI device initialized\r\n");

  while (1) {
    printk("Attempting to pull from the MIDI msgq.\r\n");
    int ret = k_msgq_get(&midi_send_msgq, &midi_rx_packet, K_FOREVER);
    if (ret == 0) {
      printk("Valid message found, sending over USB now.\r\n");
      struct midi_ump ump_packet =
          construct_packet(2, 0, 0xB, midi_rx_packet.midi_channel,
                           MIDI_CONTROLLER, midi_rx_packet.midi_value);
      if (k_mutex_lock(&usb_mutex, K_FOREVER) == 0) {
        int usbd_ret = usbd_midi_send(mideej_midi, ump_packet);
        if (usbd_ret != 0) {
          printk("USB MIDI packet unsuccessful\r\n");
        }
        k_mutex_unlock(&usb_mutex);
      }
    };
    // sleeping may not be needed
    // k_sleep(K_MSEC(1));
  }
}
K_THREAD_DEFINE(midi_send_thread_id, MIDI_SEND_TASK_STACK_SIZE,
                midi_send_thread, NULL, NULL, NULL, MIDI_SEND_TASK_PRIORITY, 0,
                0);
