/*
 * ui.h
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "main.h"
#include "usbd_cdc_if.h"
#include "scoreboard.h"

#define UI_BUFFER_SIZE 256

void echo_terminal(scoreboard_t *s, uint8_t *rx_value);
void print_terminal(scoreboard_t *s, char *message);
void print_scoreboard(scoreboard_t *s, char *message);
void print_pc_console(scoreboard_t *s, char *message);
#endif /* INC_UI_H_ */
