/*
 * commands.h
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#ifndef INC_COMMANDS_H_
#define INC_COMMANDS_H_

#include "scoreboard.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef enum command {
    INVALID_COMMAND,
    CMD_TERMINAL_MODE,
    CMD_PC_MODE,
    CMD_SCOREBOARD_MODE,
    CMD_SET_DATE,
    CMD_SET_TIME,
    CMD_GET_DATE,
    CMD_GET_TIME,
    CMD_LIST_DEVICES,
    CMD_LIST_SCORES,
    CMD_POLLING_MODE,
    NUM_COMMANDS
} command_t;

typedef enum cmd_status {
    CMD_OK, CMD_ERROR, CMD_INVALID
} cmd_status_t;

command_t parse_command(uint8_t *command, uint8_t *token, uint8_t *parameter);
cmd_status_t execute_command(scoreboard_t *scoreboard, command_t command, uint8_t *parameter);
#endif /* INC_COMMANDS_H_ */
