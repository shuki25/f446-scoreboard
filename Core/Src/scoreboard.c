/*
 * scoreboard.c
 *
 *  Created on: Mar 12, 2024
 *      Author: josh
 */

#include "scoreboard.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "cmsis_os.h"
#include "ring_buffer.h"
#include "ui.h"
#include "commands.h"
#include "rtc.h"
#include "i2c_master.h"
#include "led_indicator.h"

extern ring_buffer_t rx_buffer;
extern I2C_HandleTypeDef hi2c1;
extern led_indicator_t serial_indicator;
extern led_indicator_t console_indicator[];
extern uint8_t delta_link;
extern uint8_t link_status[MAX_NUM_CONSOLES];

scoreboard_t scoreboard;
i2c_scoreboard_t i2c_scoreboard[MAX_NUM_CONSOLES];

/*-------------------------------------------------------------------------------------------------
 * Function: scoreboard_init
 *
 * This function will initialize the scoreboard by setting up the I2C bus
 * and the GPIO pins for the scoreboard.
 *
 * Parameters: None
 * Return: None
 *-----------------------------------------------------------------------------------------------*/
void scoreboard_init() {
    // TODO: Initialize the I2C bus
    // TODO: Initialize the GPIO pins
    scoreboard.mode = SCOREBOARD_MODE;
    scoreboard.num_consoles = MAX_NUM_CONSOLES;
    scoreboard.polling_mode = 0;
    memset(scoreboard.scores, 0, sizeof(scoreboard.scores));
    for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
        memset(&i2c_scoreboard[i], 0, sizeof(i2c_scoreboard_t));
    }
    RTC_sync_set_time(23, 59, 30); // Set the time to 23:59:00 by default to
                                   // verify midnight rollover is working properly
    RTC_sync_set_date(2024, 1, 1); // Set the date to January 1, 2024 by default
}

/*-------------------------------------------------------------------------------------------------
 * Function: scoreboard_start
 *
 * This function will start the scoreboard and begin query scores from
 * connected gaming consoles via I2C bus. It will listen for serial requests
 * from a PC and respond with the current score.
 *
 * Parameters: None
 * Return: None
 *------------------------------------------------------------------------------------------------*/
void scoreboard_start() {

    uint8_t output_buffer[256];
    uint8_t command_buffer[128];
    uint8_t command[32];
    command_t cmd_token;
    uint8_t parameter[64];
    uint8_t rx_value;
    uint8_t ring_buffer_status;
    device_list_t consoles[5];
    uint8_t scoreboard_register[REGISTERS_SIZE];
    uint16_t counter = 1000;
    uint16_t link_counter = 5;
    bool has_command = false;
    int i = 0; // command buffer index
    HAL_StatusTypeDef status;

    // Initialize the ring buffer
    ring_buffer_init(&rx_buffer, 256, sizeof(uint8_t));

    // Print banner
    //print_pc_console(&scoreboard, "Scanning for connected devices\r\n");

    // Poll I2C slaves to get a list of connected devices
    initialize_device_list(consoles);
    i2c_master_scan(&hi2c1, consoles);

    // Fetch initial scoreboard data
    for (int j = 0; j < MAX_NUM_CONSOLES; j++) {
        if (consoles[j].is_active) {
            led_indicator_set_blink(&console_indicator[j], 400, 6);
            fetch_scoreboard_data(&hi2c1, &consoles[j], scoreboard_register);
            register2struct(scoreboard_register, &i2c_scoreboard[j]);
            scoreboard.scores[j].console_id = consoles[j].device_id;
            scoreboard.scores[j].grid_size = (i2c_scoreboard[j].current_game_state3 & GAME_GRID_SIZE)
                    >> GAME_GRID_SIZE_SHIFT;
            scoreboard.scores[j].game_status = i2c_scoreboard[j].current_game_state & GAME_STATUS;
            scoreboard.scores[j].game_difficulty = (i2c_scoreboard[j].console_info & GAME_LEVEL_MODE)
                    >> GAME_LEVEL_MODE_SHIFT;
            scoreboard.scores[j].cause_of_death =
                    (i2c_scoreboard[j].current_game_state3 & GAME_CAUSE_OF_DEATH) >> GAME_CAUSE_OF_DEATH_SHIFT;
            scoreboard.scores[j].game_speed = (i2c_scoreboard[j].current_game_state2 & GAME_SPEED)
                    >> GAME_SPEED_SHIFT;
            scoreboard.scores[j].is_connected = 1;
            scoreboard.scores[j].score1 = i2c_scoreboard[j].current_score1;
            scoreboard.scores[j].score2 = i2c_scoreboard[j].current_score2;
            scoreboard.scores[j].apples1 = i2c_scoreboard[j].number_apples1;
            scoreboard.scores[j].apples2 = i2c_scoreboard[j].number_apples2;
            scoreboard.scores[j].level = (i2c_scoreboard[j].current_game_state & GAME_PLAYING_LEVEL)
                    >> GAME_PLAYING_LEVEL_SHIFT;
            scoreboard.scores[j].playing_mode = i2c_scoreboard[j].current_game_state2 & GAME_NUM_PLAYERS;
            scoreboard.scores[j].with_poison = (i2c_scoreboard[j].current_game_state2 & GAME_POISON_FLAG)
                    >> GAME_POISON_SHIFT;
        } else {
            scoreboard.scores[j].console_id = consoles[j].device_id;
            scoreboard.scores[j].is_connected = 0;
            scoreboard.scores[j].score1 = 0;
            scoreboard.scores[j].score2 = 0;
            scoreboard.scores[j].apples1 = 0;
            scoreboard.scores[j].apples2 = 0;
            scoreboard.scores[j].level = 0;
            scoreboard.scores[j].with_poison = 0;
            scoreboard.scores[j].playing_mode = 0;
            scoreboard.scores[j].game_status = 0;
        }
    }

    /* Infinite loop */
    for (;;) {
        if (delta_link || link_counter == 0) {
            i2c_master_scan(&hi2c1, consoles);
            for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
                if (link_status[i]) {
                    led_indicator_set_blink(&console_indicator[i], 400, 6);
                }
            }
            delta_link = 0;
            link_counter = 5;
        }
        if (rx_buffer.new_data) {
            rx_buffer.new_data = false;
            for (int j = rx_buffer.num_new_data - 1; j >= 0; j--) {
                rx_value = 0xFF;
                ring_buffer_status = ring_buffer_peek(&rx_buffer, j, &rx_value);
                if (ring_buffer_status == RING_BUFFER_OFFSET_OUT_OF_BOUNDS) {
                    Error_Handler();
                }
                if (rx_value == '\r') {
                    i = 0;
                    memset(command_buffer, 0, sizeof(command_buffer));
                    ring_buffer_pop(&rx_buffer, &rx_value); // Pop the newline character
                    while (!is_ring_buffer_empty(&rx_buffer)) {
                        ring_buffer_dequeue(&rx_buffer, &rx_value);
                        command_buffer[i++] = rx_value;
                    }
                    ring_buffer_flush(&rx_buffer);
                    has_command = true;
                    break;
                } else {
                    echo_terminal(&scoreboard, &rx_value);
                }
            }
            rx_buffer.num_new_data = 0;
        }

        if (has_command) {
            has_command = false;
            memset(output_buffer, 0, sizeof(output_buffer));
            cmd_token = parse_command(command_buffer, command, parameter);
            if (cmd_token == INVALID_COMMAND) {
                if (scoreboard.mode == PC_CONSOLE_MODE) {
                    sprintf((char*) output_buffer, "ERR\tInvalid command: %s\r\n", command);
                    print_pc_console(&scoreboard, (char*) output_buffer);
                } else if (scoreboard.mode == TERMINAL_CONSOLE_MODE) {
                    sprintf((char*) output_buffer, "\r\nInvalid command: %s\r\n", command);
                    print_terminal(&scoreboard, (char*) output_buffer);
                } else {
                    sprintf((char*) output_buffer, "{'error': 'Invalid command: %s', 'status': 0}\n",
                            command);
                    print_scoreboard(&scoreboard, (char*) output_buffer);
                }

            } else {
                execute_command(&scoreboard, cmd_token, parameter);
            }
        }
        osDelay(1);
        counter--;
        if (counter == 0) {
            link_counter--;
            counter = 500;
            for (int j = 0; j < MAX_NUM_CONSOLES; j++) {
                if (link_status[j]) {
                    led_indicator_set_blink(&console_indicator[j], 400, 6);
                }
                if (consoles[j].is_active) {
                    status = fetch_scoreboard_data(&hi2c1, &consoles[j], scoreboard_register);
                    if (status != HAL_OK) {
                        consoles[j].is_active = 0;
                    }
                    register2struct(scoreboard_register, &i2c_scoreboard[j]);
                    scoreboard.scores[j].console_id = consoles[j].device_id;
                    scoreboard.scores[j].grid_size = (i2c_scoreboard[j].current_game_state3 & GAME_GRID_SIZE)
                            >> GAME_GRID_SIZE_SHIFT;
                    scoreboard.scores[j].game_status = i2c_scoreboard[j].current_game_state & GAME_STATUS;
                    scoreboard.scores[j].game_difficulty = (i2c_scoreboard[j].console_info & GAME_LEVEL_MODE)
                            >> GAME_LEVEL_MODE_SHIFT;
                    scoreboard.scores[j].cause_of_death = (i2c_scoreboard[j].current_game_state3
                            & GAME_CAUSE_OF_DEATH) >> GAME_CAUSE_OF_DEATH_SHIFT;
                    scoreboard.scores[j].game_speed = (i2c_scoreboard[j].current_game_state2 & GAME_SPEED)
                            >> GAME_SPEED_SHIFT;
                    scoreboard.scores[j].is_connected = 1;
                    scoreboard.scores[j].score1 = i2c_scoreboard[j].current_score1;
                    scoreboard.scores[j].score2 = i2c_scoreboard[j].current_score2;
                    scoreboard.scores[j].apples1 = i2c_scoreboard[j].number_apples1;
                    scoreboard.scores[j].apples2 = i2c_scoreboard[j].number_apples2;
                    scoreboard.scores[j].level = (i2c_scoreboard[j].current_game_state & GAME_PLAYING_LEVEL)
                            >> GAME_PLAYING_LEVEL_SHIFT;
                    scoreboard.scores[j].playing_mode = i2c_scoreboard[j].current_game_state2
                            & GAME_NUM_PLAYERS;
                    scoreboard.scores[j].with_poison = (i2c_scoreboard[j].current_game_state2
                            & GAME_POISON_FLAG) >> GAME_POISON_SHIFT;
                } else {
                    scoreboard.scores[j].console_id = consoles[j].device_id;
                    scoreboard.scores[j].is_connected = 0;
                    scoreboard.scores[j].score1 = 0;
                    scoreboard.scores[j].score2 = 0;
                    scoreboard.scores[j].apples1 = 0;
                    scoreboard.scores[j].apples2 = 0;
                    scoreboard.scores[j].level = 0;
                    scoreboard.scores[j].with_poison = 0;
                    scoreboard.scores[j].playing_mode = 0;
                    scoreboard.scores[j].game_status = 0;
                }
            }
            if (scoreboard.polling_mode) {
                cmd_token = CMD_LIST_SCORES;
                memset(parameter, 0, sizeof(parameter));
                execute_command(&scoreboard, cmd_token, parameter);
            }
        }
        osDelay(1);
    }

}
