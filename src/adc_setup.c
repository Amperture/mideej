#include "adc_setup.h"
#include "syscalls/device.h"
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

// https://www.ti.com/lit/ds/symlink/ads7830.pdf?ts=1790649479286
// Currently using the ADS7830 in single-ended mode. Channel selection
// is a little wonky, but this rearranges the bits as necessary.
// Run it with an OR operation to bitmask against a command byte.
#define ADS7830_SE_CHANNEL_SELECT(x) (uint8_t)(((x & 1) << 2 | (x >> 1)) << 4)

#define ADS7830_SINGLE_ENDED_MODE (uint8_t)(1u << 7)

#define ADS7830_POWER_DOWN_INTREF_AND_ADC 0
#define ADS7830_POWER_DOWN_INTREF (uint8_t)(1u << 2)
#define ADS7830_POWER_DOWN_ADC (uint8_t)(2u << 2)

// Set up I2C device.
static const struct i2c_dt_spec ads7830 =
    I2C_DT_SPEC_GET(DT_NODELABEL(generic_i2c_dev));

uint8_t adc_setup() {
  // Verify the hardware bus is ready
  if (!i2c_is_ready_dt(&ads7830)) {
    printk("ADC is not ready\r\n");
    return 1;
  }
  return 0;
}

uint8_t adc_se_read_channel(uint8_t ch, uint8_t *p_value) {
  uint8_t command_byte = ADS7830_SE_CHANNEL_SELECT(ch) |
                         ADS7830_SINGLE_ENDED_MODE |
                         ADS7830_POWER_DOWN_INTREF_AND_ADC;

  if (i2c_write_read_dt(&ads7830, &command_byte, 1, p_value, 1) != 0) {
    return 1;
  }
  return 0;
}

// TODO: complete this function
uint8_t adc_se_read_sequence(uint8_t *arr_ch, uint8_t *arr_values,
                             uint8_t len) {

  for (uint8_t i = 0; i < len; i++) {
    if (adc_se_read_channel(arr_ch[i], &arr_values[i]) != 0) {
      printk("Something went wrong reading ADC channel %d", arr_ch[i]);
      return 1;
    };
  }
  return 0;
}
