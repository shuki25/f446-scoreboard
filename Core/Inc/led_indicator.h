/*
 * led_indicator.h
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#ifndef INC_LED_INDICATOR_H_
#define INC_LED_INDICATOR_H_

#include "main.h"

typedef enum {
    LED_OFF, LED_ON, LED_BLINK
} led_state_t;

typedef struct {
    led_state_t state;
    GPIO_TypeDef *port;
    uint16_t pin;
    uint16_t blink_counter;
    uint32_t blink_period;
    uint32_t last_update;
} led_indicator_t;

void led_indicator_init(led_indicator_t *led, GPIO_TypeDef *port, uint16_t pin);
void led_indicator_set_state(led_indicator_t *led, led_state_t state);
void led_indicator_set_blink(led_indicator_t *led, uint32_t period, uint16_t count);
void led_indicator_update(led_indicator_t *led, uint32_t time);
void led_indicator_update_all(led_indicator_t *led[], uint8_t num_leds, uint32_t time);

#endif /* INC_LED_INDICATOR_H_ */
