/*
 * ui.c
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#include "ui.h"
#include "cmsis_os.h"


/*-----------------------------------------------------------------------------
 * Function: echo_terminal
 *
 * This function will echo a character to the terminal. Other modes will ignore.
 *
 * Parameters: scoreboard_t *s - pointer to the scoreboard
 *             uint8_t *rx_value - character to echo
 * Return: None
 *---------------------------------------------------------------------------*/
void echo_terminal(scoreboard_t *s, uint8_t *rx_value) {
    if (s->mode == TERMINAL_CONSOLE_MODE) {
        osDelay(5);
        CDC_Transmit_FS(rx_value, 1);
    }
}

/*-----------------------------------------------------------------------------
 * Function: print_terminal
 *
 * This function will print a message to the terminal. Other modes will ignore.
 *
 * Parameters: scoreboard_t *s - pointer to the scoreboard
 *             char *message - message to print
 * Return: None
 *---------------------------------------------------------------------------*/
void print_terminal(scoreboard_t *s, char *message) {
    if (s->mode == TERMINAL_CONSOLE_MODE) {
        osDelay(5);
        CDC_Transmit_FS((uint8_t*) message, strlen(message));
    }
}

/*-----------------------------------------------------------------------------
 * Function: print_scoreboard
 *
 * This function will print a message to the scoreboard. Other modes will ignore.
 *
 * Parameters: scoreboard_t *s - pointer to the scoreboard
 *             char *message - message to print
 * Return: None
 *---------------------------------------------------------------------------*/
void print_scoreboard(scoreboard_t *s, char *message) {
    if (s->mode == SCOREBOARD_MODE) {
        osDelay(5);
        CDC_Transmit_FS((uint8_t*) message, strlen(message));
    }
}

/*-----------------------------------------------------------------------------
 * Function: print_pc_console
 *
 * This function will print a message to the PC console. Other modes will ignore.
 *
 * Parameters: scoreboard_t *s - pointer to the scoreboard
 *             char *message - message to print
 * Return: None
 *---------------------------------------------------------------------------*/
void print_pc_console(scoreboard_t *s, char *message) {
    if (s->mode == PC_CONSOLE_MODE) {
        osDelay(5);
        CDC_Transmit_FS((uint8_t*) message, strlen(message));
    }
}
