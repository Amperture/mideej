#ifndef MIDI_SETUP_H
#define MIDI_SETUP_H
#include "zephyr/kernel.h"
#include <stdint.h>
#include <zephyr/usb/class/usbd_midi2.h>

struct midi_ump construct_packet(uint8_t type, uint8_t group, uint8_t command,
                                 uint8_t channel, uint8_t controller,
                                 uint8_t value);

extern struct k_msgq midi_send_msgq;

struct device *midi_setup(void);

struct midi_send_value {
  // Intended to be valid MIDI Channel
  uint8_t midi_channel;

  // Intended to be valid 7-bit MIDI CC data
  uint8_t midi_value;
};

#endif // MIDI_SETUP_H
