/*
 * led_indicator.c
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#include "led_indicator.h"


/*-----------------------------------------------------------------------------
 * Function: led_indicator_init
 *
 * This function will initialize the LED indicator
 *
 * Parameters: led_indicator_t *led - pointer to the LED indicator
 *             GPIO_TypeDef *port - GPIO port
 *             uint16_t pin - GPIO pin
 *
 * Return: None
 *---------------------------------------------------------------------------*/
void led_indicator_init(led_indicator_t *led, GPIO_TypeDef *port, uint16_t pin) {
    led->port = port;
    led->pin = pin;
    led->state = LED_OFF;
    led->blink_period = 0;
    led->blink_counter = 0;
    led->last_update = 0;
}

/*-----------------------------------------------------------------------------
 * Function: led_indicator_set_state
 *
 * This function will set the state of the LED indicator
 *
 * Parameters: led_indicator_t *led - pointer to the LED indicator
 *             led_state_t state - state of the LED
 *
 * Return: None
 *---------------------------------------------------------------------------*/
void led_indicator_set_state(led_indicator_t *led, led_state_t state) {
    HAL_GPIO_WritePin(led->port, led->pin, state);
}

/*-----------------------------------------------------------------------------
 * Function: led_indicator_set_blink
 *
 * This function will set the LED indicator to blink
 *
 * Parameters: led_indicator_t *led - pointer to the LED indicator
 *             uint32_t period - period of the blink
 *             uint16_t count - number of blinks
 *
 * Return: None
 *---------------------------------------------------------------------------*/
void led_indicator_set_blink(led_indicator_t *led, uint32_t period, uint16_t count) {
    if (count == 0) {
        count = 2;
    }
    led->blink_period = period;
    led->blink_counter += count;
    led->last_update = TIM5->CNT;
    led->state = LED_BLINK;
}

/*-----------------------------------------------------------------------------
 * Function: led_indicator_update
 *
 * This function will update the LED indicator
 *
 * Parameters: led_indicator_t *led - pointer to the LED indicator
 *             uint32_t time - current time
 *
 * Return: None
 *---------------------------------------------------------------------------*/
void led_indicator_update(led_indicator_t *led, uint32_t time) {
    if (led->state == LED_BLINK && led->blink_counter > 0) {
        if (time - led->last_update >= led->blink_period) {
            led->last_update = time;
            HAL_GPIO_TogglePin(led->port, led->pin);
            if (led->blink_counter > 0) {
                led->blink_counter--;
                if (led->blink_counter == 0) {
                    led->state = LED_OFF;
                    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET); // High turns off LED
                }
            }
        }
    }
}

void led_indicator_update_all(led_indicator_t *led[], uint8_t num_leds, uint32_t time) {
    for (int i = 0; i < num_leds; i++) {
        led_indicator_update(led[i], time);
    }
}
