#include "adc_setup.h"
#include "config.h"
#include "midi_setup.h"
#include "usb_setup.h"
#include "zephyr/audio/midi.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
// #include <zephyr/usb/usbd.h>

/* Legacy data structures
static uint8_t adc_channels_used[] = {0, 1};
static uint8_t adc_channel_values[] = {0, 0};
static uint8_t adc_read_val = 0;
*/

int main(void) {
  /* TODO: re-enable USB and MIDI once ADC threads are working
  if (usb_setup() != 0) {
    printk("Something went wrong with USB setup!\r\n");
  }
  const struct device *mideej_midi = midi_setup();
  */

  if (adc_setup() != 0) {
    printk("Failure to initialize ADC\r\n");
    return 1;
  }
  printk("thread is running...");
  return 0;
  /* legacy loop hold onto this
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
  */
}
