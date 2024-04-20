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
uint32_t random_seed = 3;


/*-------------------------------------------------------------------------------------------------
 * Function: time_elapsed
 *
 * This function will calculate the time elapsed between the previous time and the current time.
 * It will take into account the rollover of the timer.
 *
 * Parameters: uint32_t previous_time - the previous time
 *
 * Return: uint32_t - the time elapsed
 *-----------------------------------------------------------------------------------------------*/
uint32_t time_elapsed(uint32_t previous_time) {
    uint32_t current_time = TIM5->CNT;
    if (current_time < previous_time) {
        return 0xFFFFFFFF - previous_time + current_time;
    } else {
        return current_time - previous_time;
    }
}

/*-------------------------------------------------------------------------------------------------
 * Function: rng_get
 *
 * This function will generate a random number based on the current time and the seed.
 *
 * Parameters: uint32_t max_value - the maximum value of the random number
 * Return: uint32_t - the random number generated
 *-----------------------------------------------------------------------------------------------*/
uint32_t rng_get(uint32_t max_value) {
    uint32_t counter = TIM5->CNT;
    for (uint32_t i = 0; i < 32; i++) {
        asm("nop");
    }
    counter ^= TIM2->CNT;
    random_seed = random_seed * 1103515245 + 12345;
    counter += random_seed;

    return counter % max_value;
}

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
    memset(&scoreboard, 0, sizeof(scoreboard_t));
    scoreboard.mode = SCOREBOARD_MODE;
    scoreboard.num_consoles = MAX_NUM_CONSOLES;
    scoreboard.polling_mode = 0;
    scoreboard.demo_mode = 0;
    scoreboard.is_demo_mode_initialized = 0;
    memset(scoreboard.scores, 0, sizeof(scoreboard.scores));
    for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
        memset(&i2c_scoreboard[i], 0, sizeof(i2c_scoreboard_t));
    }
    RTC_sync_set_time(23, 59, 30); // Set the time to 23:59:00 by default to
                                   // verify midnight rollover is working properly
    RTC_sync_set_date(2024, 1, 1); // Set the date to January 1, 2024 by default
}

/*-------------------------------------------------------------------------------------------------
 * Function: scoreboard_demo_mode_init
 *
 * This function will initialize the scoreboard in demo mode. It will set the
 * scores for each console to a random value.
 *
 * Parameters: None
 * Return: None
 *-----------------------------------------------------------------------------------------------*/
void scoreboard_demo_mode_init() {
    if (!scoreboard.is_demo_mode_initialized && scoreboard.demo_mode) {
        scoreboard.is_demo_mode_initialized = 1;
        for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
            memset(&scoreboard.scores[i], 0, sizeof(score_t));
            memset(&scoreboard.stats[i], 0, sizeof(stats_t));
            scoreboard.scores[i].console_id = i + 1;
            scoreboard.scores[i].is_connected = 1;
            scoreboard.scores[i].game_status = 1;
            scoreboard.scores[i].playing_mode = rng_get(2) ? 1 : 0;
            scoreboard.scores[i].score1 = rng_get(100);
            scoreboard.scores[i].apples1 = rng_get(10);
            if (scoreboard.scores[i].playing_mode) {
                scoreboard.scores[i].grid_size = 1;
                scoreboard.scores[i].score2 = rng_get(100);
                scoreboard.scores[i].apples2 = rng_get(10);
            } else {
                scoreboard.scores[i].grid_size = 0;
            }
            scoreboard.scores[i].playing_time = rng_get(240);
            scoreboard.scores[i].game_difficulty = rng_get(3);
            scoreboard.scores[i].cause_of_death = 0;
            scoreboard.scores[i].level = rng_get(3);
            scoreboard.scores[i].game_speed = 50 - scoreboard.scores[i].level * 5;
            scoreboard.scores[i].with_poison = rng_get(2) ? 1 : 0;
            scoreboard.stats[i].num_apples_easy = rng_get(100);
            scoreboard.stats[i].num_apples_medium = rng_get(100);
            scoreboard.stats[i].num_apples_hard = rng_get(100);
            scoreboard.stats[i].num_apples_insane = rng_get(100);
            scoreboard.stats[i].high_score_easy = rng_get(100);
            scoreboard.stats[i].high_score_medium = rng_get(100);
            scoreboard.stats[i].high_score_hard = rng_get(100);
            scoreboard.stats[i].high_score_insane = rng_get(100);
            scoreboard.stats[i].initials_easy[0] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_easy[1] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_easy[2] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_medium[0] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_medium[1] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_medium[2] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_hard[0] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_hard[1] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_hard[2] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_insane[0] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_insane[1] = 'A' + rng_get(26);
            scoreboard.stats[i].initials_insane[2] = 'A' + rng_get(26);
        }
    }
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
    uint32_t i2c_command = 0;
    uint32_t seed = 0;
    uint8_t game_ended[MAX_NUM_CONSOLES] = { 0 };
    uint8_t tournament_ended = 0;
    uint32_t previous_time = TIM5->CNT;

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
            memset(&scoreboard.scores[j], 0, sizeof(score_t));
            memset(&scoreboard.stats[j], 0, sizeof(stats_t));
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
            scoreboard.scores[j].playing_time = i2c_scoreboard[j].playing_time;
            scoreboard.scores[j].level = (i2c_scoreboard[j].current_game_state & GAME_PLAYING_LEVEL)
                    >> GAME_PLAYING_LEVEL_SHIFT;
            scoreboard.scores[j].playing_mode = i2c_scoreboard[j].current_game_state2 & GAME_NUM_PLAYERS;
            scoreboard.scores[j].with_poison = (i2c_scoreboard[j].current_game_state2 & GAME_POISON_FLAG)
                    >> GAME_POISON_SHIFT;
            scoreboard.stats[j].num_apples_easy = i2c_scoreboard[j].num_apples_easy;
            scoreboard.stats[j].num_apples_medium = i2c_scoreboard[j].num_apples_medium;
            scoreboard.stats[j].num_apples_hard = i2c_scoreboard[j].num_apples_hard;
            scoreboard.stats[j].num_apples_insane = i2c_scoreboard[j].num_apples_insane;
            scoreboard.stats[j].high_score_easy = i2c_scoreboard[j].high_score_easy;
            scoreboard.stats[j].high_score_medium = i2c_scoreboard[j].high_score_medium;
            scoreboard.stats[j].high_score_hard = i2c_scoreboard[j].high_score_hard;
            scoreboard.stats[j].high_score_insane = i2c_scoreboard[j].high_score_insane;
            strncpy(scoreboard.stats[j].initials_easy, i2c_scoreboard[j].initials_easy, 3);
            strncpy(scoreboard.stats[j].initials_medium, i2c_scoreboard[j].initials_medium, 3);
            strncpy(scoreboard.stats[j].initials_hard, i2c_scoreboard[j].initials_hard, 3);
            strncpy(scoreboard.stats[j].initials_insane, i2c_scoreboard[j].initials_insane, 3);
        } else {
            scoreboard.scores[j].console_id = consoles[j].device_id;
            scoreboard.scores[j].is_connected = 0;
            scoreboard.scores[j].score1 = 0;
            scoreboard.scores[j].score2 = 0;
            scoreboard.scores[j].apples1 = 0;
            scoreboard.scores[j].apples2 = 0;
            scoreboard.scores[j].playing_time = 0;
            scoreboard.scores[j].level = 0;
            scoreboard.scores[j].with_poison = 0;
            scoreboard.scores[j].playing_mode = 0;
            scoreboard.scores[j].game_status = 0;
        }
    }

    /* Infinite loop */
    for (;;) {
        if (delta_link || link_counter == 0) {
            // Reset I2C bus and scan for connected devices
            SYSCFG->CFGR |= SYSCFG_CFGR_FMPI2C1_SCL;
            SYSCFG->CFGR |= SYSCFG_CFGR_FMPI2C1_SDA;
            HAL_I2C_DeInit(&hi2c1);
            SYSCFG->CFGR &= ~SYSCFG_CFGR_FMPI2C1_SCL;
            SYSCFG->CFGR &= ~SYSCFG_CFGR_FMPI2C1_SDA;
            HAL_I2C_Init(&hi2c1);

            i2c_master_scan(&hi2c1, consoles);
            for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
                if (link_status[i]) {
                    led_indicator_set_blink(&console_indicator[i], 400, 6);
                }
            }
            delta_link = 0;
            link_counter = 5;
        }
        // Check for incoming new data in the ring buffer
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
            memset(command, 0, sizeof(command));
            memset(parameter, 0, sizeof(parameter));
            cmd_token = parse_command(command_buffer, command, parameter);
            if (cmd_token == INVALID_COMMAND || cmd_token == INVALID_PARAMETER_COUNT) {
                if (scoreboard.mode == PC_CONSOLE_MODE) {
                    if (cmd_token == INVALID_COMMAND)
                        sprintf((char*) output_buffer, "ERR\tInvalid command: %s\r\n", command);
                    else if (cmd_token == INVALID_PARAMETER_COUNT)
                        sprintf((char*) output_buffer, "ERR\tInvalid parameter count\r\n");
                    print_pc_console(&scoreboard, (char*) output_buffer);
                } else if (scoreboard.mode == TERMINAL_CONSOLE_MODE) {
                    if (cmd_token == INVALID_COMMAND)
                        sprintf((char*) output_buffer, "\r\nInvalid command: %s\r\n", command);
                    else if (cmd_token == INVALID_PARAMETER_COUNT)
                        sprintf((char*) output_buffer, "\r\nInvalid parameter count\r\n");
                    print_terminal(&scoreboard, (char*) output_buffer);
                } else {
                    if (cmd_token == INVALID_COMMAND)
                        sprintf((char*) output_buffer, "{'error': 'Invalid command: %s', 'status': 0}\n",
                                command);
                    else if (cmd_token == INVALID_PARAMETER_COUNT)
                        sprintf((char*) output_buffer, "{'error': 'Invalid parameter count', 'status': 0}\n");
                    print_scoreboard(&scoreboard, (char*) output_buffer);
                }

            } else {
                switch (cmd_token) {
                    case CMD_SET_SPEED:
                    case CMD_SET_LEVEL:
                    case CMD_PREPARE_GAME:
                    case CMD_START_GAME:
                    case CMD_END_GAME:
                    case CMD_PAUSE_GAME:
                    case CMD_RANDOM_SEED:
                        i2c_command = parse_i2c_command(cmd_token, parameter);
                        if (cmd_token == CMD_RANDOM_SEED) {
                            seed = atoi((char*) parameter);
                            if (seed == 0) {
                                seed = TIM2->CNT;
                            }
                        } else if (cmd_token == CMD_START_GAME) {
                            scoreboard.is_tournament_mode = 1;
                            memset(game_ended, 0, sizeof(game_ended));
                        } else {
                            seed = 0;
                        }
                        if (i2c_command != 0)
                            i2c_send_command(&hi2c1, consoles, i2c_command, seed);
                        break;
                    default:
                        execute_command(&scoreboard, cmd_token, parameter);
                        break;
                }
            }
        }
        osThreadYield();
        if (time_elapsed(previous_time) >= 10000) {
            previous_time = TIM5->CNT;
            link_counter--;
            if (scoreboard.demo_mode) {
                if (!scoreboard.is_demo_mode_initialized) {
                    scoreboard_demo_mode_init();
                }
                for (int j = 0; j < MAX_NUM_CONSOLES; j++) {
                    consoles[j].is_active = 1;
                    if (scoreboard.scores[j].game_status == 1) {
                        if (scoreboard.scores[j].score1 > 150 && rng_get(50) >= 35) {
                            scoreboard.scores[j].game_status = 3;
                            scoreboard.scores[j].cause_of_death = 1 + rng_get(4);
                        }
                        if (rng_get(20) >= 7) {
                            scoreboard.scores[j].score1 += rng_get(10) + 1;
                            scoreboard.scores[j].apples1 += rng_get(2) + 1;
                            if (scoreboard.scores[j].playing_mode) {
                                scoreboard.scores[j].score2 += rng_get(10) + 1;
                                scoreboard.scores[j].apples2 += rng_get(2) + 1;
                            }
                        }
                        scoreboard.scores[j].playing_time++;
                    } else if (scoreboard.scores[j].game_status == 3) {
                        if (rng_get(100) >= 87) {
                            scoreboard.scores[j].game_status = 1;
                            scoreboard.scores[j].playing_mode = rng_get(2) ? 1 : 0;
                            scoreboard.scores[j].score1 = rng_get(100);
                            scoreboard.scores[j].apples1 = rng_get(10);
                            scoreboard.scores[j].score2 = 0;
                            scoreboard.scores[j].apples2 = 0;
                            if (scoreboard.scores[j].playing_mode) {
                                scoreboard.scores[j].grid_size = 1;
                                scoreboard.scores[j].score2 = rng_get(100);
                                scoreboard.scores[j].apples2 = rng_get(10);
                            } else {
                                scoreboard.scores[j].grid_size = 0;
                            }
                            scoreboard.scores[j].playing_time = rng_get(120);
                            scoreboard.scores[j].game_difficulty = rng_get(3);
                            scoreboard.scores[j].cause_of_death = 0;
                            scoreboard.scores[j].level = rng_get(3);
                            scoreboard.scores[i].game_speed = 50 - scoreboard.scores[i].level * 5;
                            scoreboard.scores[j].with_poison = rng_get(2) ? 1 : 0;
                        }
                    }
                }
            } else if (!scoreboard.demo_mode && scoreboard.is_demo_mode_initialized) {
                scoreboard.is_demo_mode_initialized = 0;
                memset(scoreboard.scores, 0, sizeof(scoreboard.scores));
                memset(scoreboard.stats, 0, sizeof(scoreboard.stats));
            } else {
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
                        scoreboard.scores[j].grid_size = (i2c_scoreboard[j].current_game_state3
                                & GAME_GRID_SIZE) >> GAME_GRID_SIZE_SHIFT;
                        scoreboard.scores[j].game_status = i2c_scoreboard[j].current_game_state & GAME_STATUS;
                        scoreboard.scores[j].game_difficulty = (i2c_scoreboard[j].console_info
                                & GAME_LEVEL_MODE) >> GAME_LEVEL_MODE_SHIFT;
                        scoreboard.scores[j].cause_of_death = (i2c_scoreboard[j].current_game_state3
                                & GAME_CAUSE_OF_DEATH) >> GAME_CAUSE_OF_DEATH_SHIFT;
                        scoreboard.scores[j].game_speed = (i2c_scoreboard[j].current_game_state2 & GAME_SPEED)
                                >> GAME_SPEED_SHIFT;
                        scoreboard.scores[j].is_connected = 1;
                        scoreboard.scores[j].score1 = i2c_scoreboard[j].current_score1;
                        scoreboard.scores[j].score2 = i2c_scoreboard[j].current_score2;
                        scoreboard.scores[j].apples1 = i2c_scoreboard[j].number_apples1;
                        scoreboard.scores[j].apples2 = i2c_scoreboard[j].number_apples2;
                        scoreboard.scores[j].playing_time = i2c_scoreboard[j].playing_time;
                        scoreboard.scores[j].level = (i2c_scoreboard[j].current_game_state
                                & GAME_PLAYING_LEVEL) >> GAME_PLAYING_LEVEL_SHIFT;
                        scoreboard.scores[j].playing_mode = i2c_scoreboard[j].current_game_state2
                                & GAME_NUM_PLAYERS;
                        scoreboard.scores[j].with_poison = (i2c_scoreboard[j].current_game_state2
                                & GAME_POISON_FLAG) >> GAME_POISON_SHIFT;
                        scoreboard.stats[j].num_apples_easy = i2c_scoreboard[j].num_apples_easy;
                        scoreboard.stats[j].num_apples_medium = i2c_scoreboard[j].num_apples_medium;
                        scoreboard.stats[j].num_apples_hard = i2c_scoreboard[j].num_apples_hard;
                        scoreboard.stats[j].num_apples_insane = i2c_scoreboard[j].num_apples_insane;
                        scoreboard.stats[j].high_score_easy = i2c_scoreboard[j].high_score_easy;
                        scoreboard.stats[j].high_score_medium = i2c_scoreboard[j].high_score_medium;
                        scoreboard.stats[j].high_score_hard = i2c_scoreboard[j].high_score_hard;
                        scoreboard.stats[j].high_score_insane = i2c_scoreboard[j].high_score_insane;
                        strncpy(scoreboard.stats[j].initials_easy, i2c_scoreboard[j].initials_easy, 3);
                        strncpy(scoreboard.stats[j].initials_medium, i2c_scoreboard[j].initials_medium, 3);
                        strncpy(scoreboard.stats[j].initials_hard, i2c_scoreboard[j].initials_hard, 3);
                        strncpy(scoreboard.stats[j].initials_insane, i2c_scoreboard[j].initials_insane, 3);

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
            }
            if (scoreboard.is_tournament_mode) {
                for (int k = 0; k < MAX_NUM_CONSOLES; k++) {
                    if (consoles[k].is_active) {
                        if (scoreboard.scores[k].game_status == 3) { // game ended
                            game_ended[k] = 1;
                        }
                    }
                }

                tournament_ended = 1;
                for (int k = 0; k < MAX_NUM_CONSOLES; k++) {
                    if (game_ended[k] == 0) { // At least one console is still playing, so tournament is not over
                        tournament_ended = 0;
                    }
                }
                if (tournament_ended) {
                    scoreboard.is_tournament_mode = 0;
                }
            }

            if (scoreboard.polling_mode) {
                cmd_token = CMD_LIST_SCORES;
                memset(parameter, 0, sizeof(parameter));
                execute_command(&scoreboard, cmd_token, parameter);
            }
        }
        osThreadYield();
    }

}
