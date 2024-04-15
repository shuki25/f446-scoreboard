/*
 * i2c_master.c
 *
 *  Created on: Mar 18, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#include "main.h"
#include "game_stats.h"
#include "i2c_master.h"
#include "scoreboard.h"
#include <string.h>

volatile uint8_t i2c_rx_buffer[I2C_BUFFER_SIZE];
volatile uint8_t i2c_rx_index = 0;
volatile uint8_t i2c_tx_index = 0;
volatile uint8_t bytes_received = 0;
volatile uint8_t start_position = 0;
//volatile i2c_scoreboard_t i2c_register_struct;
volatile uint8_t i2c_register[REGISTERS_SIZE] = { 0 };

extern I2C_HandleTypeDef hi2c2;
extern RTC_HandleTypeDef hrtc;

void volatile_memcpy(volatile void *dest, void *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        *((volatile uint8_t*) dest + i) = *((uint8_t*) src + i);
    }
}

void volatile_memset(volatile void *dest, uint8_t value, size_t n) {
    for (size_t i = 0; i < n; i++) {
        *((volatile uint8_t*) dest + i) = value;
    }
}

void initialize_device_list(device_list_t list[]) {
    for (int i = 0; i < MAX_NUM_CONSOLES; i++) {
        list[i].device_id = 0;
        list[i].i2c_addr = 0;
        list[i].is_active = 0;
    }
}

void initialize_scoreboard_data(i2c_scoreboard_t *data) {
    memset(data, 0, sizeof(i2c_scoreboard_t));
}

void struct2register(i2c_scoreboard_t *data) {
    uint8_t i = 0;
    i2c_register[i++] = data->console_info;
    i2c_register[i++] = data->current_game_state;
    i2c_register[i++] = data->current_game_state2;
    i2c_register[i++] = data->current_game_state3;
    i2c_register[i++] = (uint8_t) (data->current_score1 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->current_score1 & 0xFF;
    i2c_register[i++] = (uint8_t) (data->current_score2 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->current_score2 & 0xFF;
    i2c_register[i++] = (uint8_t) (data->number_apples1 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->number_apples1 & 0xFF;
    i2c_register[i++] = (uint8_t) (data->number_apples2 >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->number_apples2 & 0xFF;
    i2c_register[i++] = (uint8_t) (data->high_score >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->high_score & 0xFF;
    i2c_register[i++] = (uint8_t) (data->playing_time >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->playing_time & 0xFF;
    i2c_register[i++] = (uint8_t) (data->num_apples_easy >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->num_apples_easy & 0xFF;
    i2c_register[i++] = (uint8_t) (data->num_apples_medium >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->num_apples_medium & 0xFF;
    i2c_register[i++] = (uint8_t) (data->num_apples_hard >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->num_apples_hard & 0xFF;
    i2c_register[i++] = (uint8_t) (data->num_apples_insane >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->num_apples_insane & 0xFF;
    i2c_register[i++] = (uint8_t) (data->high_score_easy >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->high_score_easy & 0xFF;
    i2c_register[i++] = (uint8_t) (data->high_score_medium >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->high_score_medium & 0xFF;
    i2c_register[i++] = (uint8_t) (data->high_score_hard >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->high_score_hard & 0xFF;
    i2c_register[i++] = (uint8_t) (data->high_score_insane >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->high_score_insane & 0xFF;
    i2c_register[i++] = (uint8_t) data->initials_easy[0];
    i2c_register[i++] = (uint8_t) data->initials_easy[1];
    i2c_register[i++] = (uint8_t) data->initials_easy[2];
    i2c_register[i++] = (uint8_t) data->initials_medium[0];
    i2c_register[i++] = (uint8_t) data->initials_medium[1];
    i2c_register[i++] = (uint8_t) data->initials_medium[2];
    i2c_register[i++] = (uint8_t) data->initials_hard[0];
    i2c_register[i++] = (uint8_t) data->initials_hard[1];
    i2c_register[i++] = (uint8_t) data->initials_hard[2];
    i2c_register[i++] = (uint8_t) data->initials_insane[0];
    i2c_register[i++] = (uint8_t) data->initials_insane[1];
    i2c_register[i++] = (uint8_t) data->initials_insane[2];
    i2c_register[i++] = (uint8_t) (data->date_time >> 24) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->date_time >> 16) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->date_time >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->date_time & 0xFF;
    i2c_register[i++] = (uint8_t) (data->command >> 24) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->command >> 16) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->command >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->command & 0xFF;
}

void register2struct(uint8_t reg[], i2c_scoreboard_t *data) {
    uint8_t i = 0;
    data->console_info = reg[i++];
    data->current_game_state = reg[i++];
    data->current_game_state2 = reg[i++];
    data->current_game_state3 = reg[i++];
    data->current_score1 = (reg[i++] << 8);
    data->current_score1 |= reg[i++];
    data->current_score2 = (reg[i++] << 8);
    data->current_score2 |= reg[i++];
    data->number_apples1 = (reg[i++] << 8);
    data->number_apples1 |= reg[i++];
    data->number_apples2 = (reg[i++] << 8);
    data->number_apples2 |= reg[i++];
    data->high_score = (reg[i++] << 8);
    data->high_score |= reg[i++];
    data->playing_time = (reg[i++] << 8);
    data->playing_time |= reg[i++];
    data->num_apples_easy = (reg[i++] << 8);
    data->num_apples_easy |= reg[i++];
    data->num_apples_medium = (reg[i++] << 8);
    data->num_apples_medium |= reg[i++];
    data->num_apples_hard = (reg[i++] << 8);
    data->num_apples_hard |= reg[i++];
    data->num_apples_insane = (reg[i++] << 8);
    data->num_apples_insane |= reg[i++];
    data->high_score_easy = (reg[i++] << 8);
    data->high_score_easy |= reg[i++];
    data->high_score_medium = (reg[i++] << 8);
    data->high_score_medium |= reg[i++];
    data->high_score_hard = (reg[i++] << 8);
    data->high_score_hard |= reg[i++];
    data->high_score_insane = (reg[i++] << 8);
    data->high_score_insane |= reg[i++];
    memcpy(data->initials_easy, (char*) &reg[i], 3);
    i += 3;
    memcpy(data->initials_medium, (char*) &reg[i], 3);
    i += 3;
    memcpy(data->initials_hard, (char*) &reg[i], 3);
    i += 3;
    memcpy(data->initials_insane, (char*) &reg[i], 3);
    i += 3;
    data->date_time = (reg[i++] << 24);
    data->date_time |= (reg[i++] << 16);
    data->date_time |= (reg[i++] << 8);
    data->date_time |= reg[i++];
    // Clear out command since it's write only
    data->command = 0;
}

void update_command_register(i2c_scoreboard_t *data, uint32_t command) {
    data->command = command;
    uint8_t i = 0x30;
    i2c_register[i++] = (uint8_t) (data->command >> 24) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->command >> 16) & 0xFF;
    i2c_register[i++] = (uint8_t) (data->command >> 8) & 0xFF;
    i2c_register[i++] = (uint8_t) data->command & 0xFF;
}

void update_register(i2c_scoreboard_t *data, game_stats_t game_stats[], uint16_t current_score[],
        uint16_t best_score, uint16_t number_apples[], uint8_t level, uint8_t game_in_progress,
        uint8_t game_pause, uint8_t game_over, uint8_t game_pace, uint8_t clock_sync_flag,
        uint32_t game_elapsed_time, game_options_t *game_options, grid_size_options_t grid_size_options) {

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    uint8_t console_id = ((I2C2->OAR1 & 0x7E) >> 1) - 15;
    data->console_info = console_id | CONSOLE_SIGNATURE | (clock_sync_flag << CONSOLE_CLOCK_SHIFT)
            | (((uint8_t) game_options->difficulty & 0x0F) << GAME_LEVEL_MODE_SHIFT);

    uint8_t game_progress = 0;
    if (game_in_progress) {
        game_progress = 1;
    } else if (game_pause) {
        game_progress = 2;
    } else if (game_over) {
        game_progress = 3;
    }

    data->current_game_state = (game_progress & 0x03) | (level << GAME_PLAYING_LEVEL_SHIFT);
    data->current_game_state2 = (uint8_t) game_options->num_players | (game_pace << GAME_SPEED_SHIFT)
            | ((uint8_t) game_options->poison << GAME_POISON_SHIFT);
    data->current_game_state3 = (game_over & GAME_CAUSE_OF_DEATH)
            | ((grid_size_options << GAME_GRID_SIZE_SHIFT) & GAME_GRID_SIZE);
    data->current_score1 = current_score[0];
    data->current_score2 = current_score[1];
    data->number_apples1 = number_apples[0];
    data->number_apples2 = number_apples[1];
    data->high_score = best_score;
    data->playing_time = (uint16_t) game_elapsed_time;
    data->num_apples_easy = game_stats[EASY].num_apples_eaten;
    data->num_apples_medium = game_stats[MEDIUM].num_apples_eaten;
    data->num_apples_hard = game_stats[HARD].num_apples_eaten;
    data->num_apples_insane = game_stats[INSANE].num_apples_eaten;
    data->high_score_easy = game_stats[EASY].high_score;
    data->high_score_medium = game_stats[MEDIUM].high_score;
    data->high_score_hard = game_stats[HARD].high_score;
    data->high_score_insane = game_stats[INSANE].high_score;
    memcpy(data->initials_easy, game_stats[EASY].player_name, 3);
    memcpy(data->initials_medium, game_stats[MEDIUM].player_name, 3);
    memcpy(data->initials_hard, game_stats[HARD].player_name, 3);
    memcpy(data->initials_insane, game_stats[INSANE].player_name, 3);

    data->date_time = (sDate.Year << YEAR_SHIFT) | (sDate.Month << MONTH_SHIFT) | (sDate.Date << DAY_SHIFT)
            | (sTime.Hours << HOUR_SHIFT) | (sTime.Minutes << MINUTE_SHIFT) | (sTime.Seconds << SECOND_SHIFT);
    struct2register(data);
}

void process_data(i2c_scoreboard_t *data) {
    // TODO: Implement this function
    if (bytes_received > 0) {
        if (i2c_rx_buffer[0] == 0x05) {
            data->current_score2 = (i2c_rx_buffer[1] << 8) | i2c_rx_buffer[2];
        }
        bytes_received = 0;
        i2c_rx_index = 0;
        i2c_tx_index = 0;
        struct2register(data);
    }
}

HAL_StatusTypeDef get_console_data(I2C_HandleTypeDef *hi2c, uint16_t i2c_addr, uint8_t reg_addr,
        uint8_t *data, uint8_t len) {
    HAL_StatusTypeDef status;
    data[0] = reg_addr;
    status = HAL_I2C_Master_Transmit(hi2c, i2c_addr, data, 1, I2C_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_I2C_Master_Receive(hi2c, i2c_addr, data, len, I2C_TIMEOUT);
        return status;
    } else {
        return status;
    }
}

HAL_StatusTypeDef fetch_scoreboard_data(I2C_HandleTypeDef *hi2c, device_list_t *device,
        uint8_t scoreboard_data[]) {
    HAL_StatusTypeDef status;
    uint8_t reg_addr[1] = { 0 };
    status = HAL_I2C_Master_Transmit(hi2c, device->i2c_addr << 1, reg_addr, 1, I2C_TIMEOUT);
    if (status == HAL_OK) {
        status = HAL_I2C_Master_Receive(hi2c, device->i2c_addr << 1, scoreboard_data, REGISTERS_SIZE,
        I2C_TIMEOUT);
    }
    return status;
}

HAL_StatusTypeDef i2c_send_command(I2C_HandleTypeDef *hi2c, device_list_t device[], uint32_t command) {
    HAL_StatusTypeDef status;
    uint8_t data[4] = { 0 };
    data[0] = (uint8_t) (command >> 24) & 0xFF;
    data[1] = (uint8_t) (command >> 16) & 0xFF;
    data[2] = (uint8_t) (command >> 8) & 0xFF;
    data[3] = (uint8_t) command & 0xFF;

    for (uint8_t i = 0; i < MAX_NUM_CONSOLES; i++) {
        if (device[i].is_active) {
            status = HAL_I2C_Mem_Write(hi2c, device[i].i2c_addr << 1, 0x30, sizeof(uint8_t), data, 4, I2C_TIMEOUT);
            if (status != HAL_OK) {
                __NOP(); // Ignore error, continue to next device
            }
        }
    }
    return status;
}

HAL_StatusTypeDef i2c_master_scan(I2C_HandleTypeDef *hi2c, device_list_t device[]) {
    HAL_StatusTypeDef status;
    uint16_t device_addr;
    uint8_t device_index;
    uint8_t data[2] = { 0, 0 };

    for (device_addr = 0x10; device_addr <= 0x14; device_addr++) {
        status = HAL_I2C_IsDeviceReady(hi2c, device_addr << 1, 1, 10);
        if (status == HAL_OK) {
            memset(&data, 0, 2);
            status = get_console_data(hi2c, device_addr << 1, 0, data, 1);
            if (status == HAL_OK) {
                if (data[0] & CONSOLE_SIGNATURE) {
                    device_index = device_addr - 16;
                    device[device_index].i2c_addr = device_addr;
                    device[device_index].device_id = data[0] & CONSOLE_IDENTIFIER;
                    device[device_index].is_active = 1;
                }
            }
        }
    }
    return status;
}

