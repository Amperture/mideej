#include "adc_setup.h"
#include "config.h"
#include "midi_setup.h"
#include <stdint.h>
#include <stdlib.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
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

K_MUTEX_DEFINE(adc_mutex);

// Set up I2C device.
static const struct i2c_dt_spec ads7830 =
    I2C_DT_SPEC_GET(DT_NODELABEL(generic_i2c_dev));

uint8_t adc_se_read_channel(uint8_t ch, uint8_t *p_value) {
  uint8_t command_byte = ADS7830_SE_CHANNEL_SELECT(ch) |
                         ADS7830_SINGLE_ENDED_MODE |
                         ADS7830_POWER_DOWN_INTREF_AND_ADC;

  if (i2c_write_read_dt(&ads7830, &command_byte, 1, p_value, 1) != 0) {
    return 1;
  }
  return 0;
}

// Deliberately overengineered for now. Mutex technically not necessary since
// this task is the only one handling the ADC, but using the mutex anyway
// as a learning experience.
uint8_t adc_channels_list[] = {ADC_CHANNELS_USED};
struct adc_read_value {
  uint8_t index;
  uint8_t channel;
  uint8_t value;
};
K_MSGQ_DEFINE(adc_read_msgq, sizeof(struct adc_read_value), 64, 1);
void adc_read_thread(void *p1, void *p2, void *p3) {
  printk("ADC Read Thread Initializing\r\n");
  struct adc_read_value tx_packet;

  if (adc_setup() != 0) {
    printk("Failure to initialize ADC\r\n");
    return;
  }

  // Make sure the adc is set up
  while (1) {
    if (k_mutex_lock(&adc_mutex, K_FOREVER) == 0) {
      for (uint8_t i = 0; i < ARRAY_SIZE(adc_channels_list); i++) {
        tx_packet.index = i;
        tx_packet.channel = adc_channels_list[i];
        adc_se_read_channel(tx_packet.channel, &tx_packet.value);
        int ret = k_msgq_put(&adc_read_msgq, &tx_packet, K_NO_WAIT);
        if (ret != 0) {
          printk("ADC Raw Queue full.\r\n");
        }
      }
      k_mutex_unlock(&adc_mutex);
    }
    // K_NO_WAIT: if queue is full, skip or handle immediately
    // May not be needed, if the midi parse thread blocks and is higher
    // priority.
    k_sleep(K_MSEC(2));
  }
}
K_THREAD_DEFINE(adc_read_thread_id, ADC_READ_TASK_STACK_SIZE, adc_read_thread,
                NULL, NULL, NULL, ADC_READ_TASK_PRIORITY, 0, 0);

struct midi_channel_value {
  uint8_t channel;
  uint8_t value;
};
void adc_parse_thread(void *p1, void *p2, void *p3) {
  printk("ADC Parse Thread Initializing\r\n");
  struct adc_read_value rx_packet;
  static struct midi_send_value tx_packet;
  static struct midi_send_value last_sent[ARRAY_SIZE(adc_channels_list)];

  struct midi_channel_value held_values[ARRAY_SIZE(adc_channels_list)];

  // Initialize the last_sent_midi_values to 200
  for (uint8_t i = 0; i < ARRAY_SIZE(adc_channels_list); i++) {
    // valid MIDI CC 1.0 values only go from 0-127, if we set initial values
    // to something outside that range, we can guarantee the first
    // value read on startup gets sent. Hence initializing to 200

    held_values[i].channel = adc_channels_list[i];
    held_values[i].value = 200;
    last_sent[i].midi_channel = adc_channels_list[i];
    last_sent[i].midi_value = 200;
  }

  while (1) {
    // Attempt to clean up and send current values if the queue was full on
    // first send
    for (uint8_t i = 0; i < ARRAY_SIZE(adc_channels_list); i++) {
      if ((held_values[i].value >> 1) != last_sent[i].midi_value) {
        tx_packet.midi_value = held_values[i].value >> 1;
        tx_packet.midi_channel = held_values[i].channel;
        int ret = k_msgq_put(&midi_send_msgq, &tx_packet, K_NO_WAIT);
        if (ret == 0) {
          last_sent[i].midi_channel = tx_packet.midi_channel;
          last_sent[i].midi_value = tx_packet.midi_value;
        } else {
          printk("MIDI Send Packet Queue _STILL_ full.\r\n");
        }
      }
    };

    int ret = k_msgq_get(&adc_read_msgq, &rx_packet, K_FOREVER);
    if (ret == 0) {

      uint8_t current_val = held_values[rx_packet.index].value;

      // Apply a Deadband and Hysteresis
      if (rx_packet.value > current_val + ADC_DEADBAND) {
        held_values[rx_packet.index].value = rx_packet.value - ADC_DEADBAND;
        printk("New Value for Channel %d: %d\r\n", rx_packet.channel,
               rx_packet.value - ADC_DEADBAND);
      } else if (rx_packet.value < current_val - ADC_DEADBAND) {
        held_values[rx_packet.index].value = rx_packet.value + ADC_DEADBAND;
        printk("New Value for Channel %d: %d\r\n", rx_packet.channel,
               rx_packet.value + ADC_DEADBAND);
      }

      if ((held_values[rx_packet.index].value >> 1) !=
          last_sent[rx_packet.index].midi_value) {
        // Prepare to send TX Packet to USB thread.
        tx_packet.midi_value = held_values[rx_packet.index].value >> 1;
        tx_packet.midi_channel = held_values[rx_packet.index].channel;

        // Send TX thread to MIDI packet. If successful, record last_sent
        int ret = k_msgq_put(&midi_send_msgq, &tx_packet, K_NO_WAIT);
        if (ret == 0) {
          last_sent[rx_packet.index].midi_channel = tx_packet.midi_channel;
          last_sent[rx_packet.index].midi_value = tx_packet.midi_value;
        } else {
          printk("MIDI Send Packet Queue full.\r\n");
        }
      }
    };
    // May not be needed, if the midi parse thread blocks and is higher
    // priority. k_sleep(K_MSEC(1));
  }
}
K_THREAD_DEFINE(adc_parse_thread_id, ADC_PARSE_TASK_STACK_SIZE,
                adc_parse_thread, NULL, NULL, NULL, ADC_PARSE_TASK_PRIORITY, 0,
                0);

uint8_t adc_setup() {
  // Verify the hardware bus is ready
  printk("initializing adc\r\n");
  if (!i2c_is_ready_dt(&ads7830)) {
    printk("ADC is not ready\r\n");
    return 1;
  }
  return 0;
}
