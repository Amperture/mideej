#ifndef ADC_SETUP_H
#define ADC_SETUP_H
#include <stdint.h>

uint8_t adc_setup();
uint8_t adc_se_read_channel(uint8_t ch, uint8_t *p_value);

#endif // ADC_SETUP_H
