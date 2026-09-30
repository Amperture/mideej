#include "adc_setup.h"
#include "midi_setup.h"
#include "usb_setup.h"
#include "zephyr/audio/midi.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usbd.h>

static uint8_t adc_channels_used[] = {0, 1};
static uint8_t adc_channel_values[] = {0, 0};
static uint8_t adc_read_val = 0;

int main(void) {

  if (usb_setup() != 0) {
    printk("Something went wrong with USB setup!\r\n");
  }

  if (adc_setup() != 0) {
    printk("Something went wrong with ADC setup!\r\n");
  }
  const struct device *mideej_midi = midi_setup();

  uint8_t channel_loop_len =
      sizeof(adc_channels_used) / sizeof(adc_channels_used[0]);

  if (channel_loop_len !=
      (sizeof(adc_channel_values) / sizeof(adc_channel_values[0]))) {
    printk("ERR: ADC channels list and values must have same length");
    return 1;
  }

  while (true) {
    for (uint8_t i = 0; i < channel_loop_len; i++) {

      if (adc_se_read_channel(adc_channels_used[i], &adc_read_val) != 0) {
        printk("something went wrong reading the ADC");
        return 1;
      }

      struct midi_ump ump_packet = construct_packet(
          2, 0, 0xB, adc_channels_used[i], 7, adc_read_val >> 1);
      usbd_midi_send(mideej_midi, ump_packet);

      k_sleep(K_MSEC(10));
    }
  }
}
