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
  uint8_t command_byte = 0x84;

  while (true) {
    int err = i2c_write_read_dt(&ads7830, &command_byte, 1, &adc_read_val, 1);
    printk("i2c error: %d\r\n", err);
    printk("ADC READ VALUE RAW: %d \r\n", adc_read_val);
    struct midi_ump ump_packet =
        construct_packet(2, 0, 0xB, 0, 7, adc_read_val);
    usbd_midi_send(mideej_midi, ump_packet);
    k_sleep(K_MSEC(100));
  }
}
