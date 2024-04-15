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
    INVALID_PARAMETER_COUNT,
    CMD_TERMINAL_MODE,
    CMD_PC_MODE,
    CMD_SCOREBOARD_MODE,
    CMD_SET_DATE, // parameter is date in format YYYY-MM-DD
    CMD_SET_TIME, // parameter is time in format HH:MM:SS
    CMD_GET_DATE,
    CMD_GET_TIME,
    CMD_LIST_DEVICES,
    CMD_LIST_SCORES,
    CMD_POLLING_MODE, // parameter is on, off
    CMD_DEMO_MODE, // parameter is on, off, status, reset
    CMD_STATS,
    CMD_SET_SPEED, // parameter is between 0-60
    CMD_SET_LEVEL, // parameter is between 0-3
    CMD_PREPARE_GAME, // parameter is level 0-3, with_poison 0-1
    CMD_START_GAME, // parameter is speed 0-60
    CMD_END_GAME,
    CMD_PAUSE_GAME,
    NUM_COMMANDS
} command_t;

typedef enum cmd_status {
    CMD_OK, CMD_ERROR, CMD_INVALID
} cmd_status_t;

command_t parse_command(uint8_t *command, uint8_t *token, uint8_t *parameter);
cmd_status_t execute_command(scoreboard_t *scoreboard, command_t command, uint8_t *parameter);
uint32_t parse_i2c_command(command_t cmd_token, uint8_t *parameter);
#endif /* INC_COMMANDS_H_ */
