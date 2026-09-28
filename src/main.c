#include "midi_setup.h"
#include "usb_setup.h"
#include "zephyr/audio/midi.h"
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usbd.h>

// Set up I2C device.
static const struct i2c_dt_spec ads7830 =
    I2C_DT_SPEC_GET(DT_NODELABEL(generic_i2c_dev));

int main(void) {

  if (usb_setup() != 0) {
    printk("Something went wrong with USB setup!\r\n");
  }
  const struct device *mideej_midi = midi_setup();

  // Verify the hardware bus is ready
  if (!i2c_is_ready_dt(&ads7830)) {
    printk("ADC is not ready\r\n");
    return 1;
  }
  uint8_t adc_read_val = 0x5A;
  uint8_t max_pin = 1;

  while (true) {

    for (uint8_t adc_pin = 0x00; adc_pin <= max_pin; adc_pin++) {
      // Default Command Byte: Single-Ended, ADC on, Internal Reference Off
      // Default to Pin 0
      uint8_t pin_sel = ((adc_pin & 1) << 2 | (adc_pin >> 1));
      uint8_t command_byte = 0x84 | (pin_sel << 4);

      int err = i2c_write_read_dt(&ads7830, &command_byte, 1, &adc_read_val, 1);
      printk("i2c error pin: %d\r\n", err);
      printk("ADC PIN %d READ VALUE RAW: %d \r\n", adc_pin, adc_read_val);
      struct midi_ump ump_packet =
          construct_packet(2, 0, 0xB, adc_pin, 7, adc_read_val >> 1);
      usbd_midi_send(mideej_midi, ump_packet);
      k_sleep(K_MSEC(100));
    }
  }
}
