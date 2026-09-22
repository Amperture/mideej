#ifndef MIDI_SETUP_H
#define MIDI_SETUP_H
#include <stdint.h>
#include <zephyr/usb/class/usbd_midi2.h>

struct midi_ump construct_packet(uint8_t type, uint8_t group, uint8_t command,
                                 uint8_t channel, uint8_t controller,
                                 uint8_t value);

struct device *midi_setup(void);
#endif // MIDI_SETUP_H
