/*
 * scoreboard.h
 *
 *  Created on: Mar 12, 2024
 *      Author: josh
 */

#ifndef INC_SCOREBOARD_H_
#define INC_SCOREBOARD_H_

#define MAX_NUM_CONSOLES 5

#include "main.h"
#include "serial.h"
#include "ring_buffer.h"
#include "game_stats.h"

// Bit Definitions for the console_info
#define CONSOLE_SIGNATURE   (0b11000000) // Fixed signature bits
#define CONSOLE_IDENTIFIER  (0b00000111) // Between 1 and 5 (I2C addresses 0x10 - 0x14)
#define CONSOLE_CLOCK_SYNC  (0b00001000) // 0 = no, 1 = yes
#define CONSOLE_CLOCK_SHIFT (3)
#define GAME_LEVEL_MODE     (0b00110000) // 0 = easy, 1 = medium, 2 = hard, 3 = insane)
#define GAME_LEVEL_MODE_SHIFT    (4)


// Bit Definitions for the current_game_state
#define GAME_STATUS         (0b00000011) // 0 = stopped, 1 = running, 2 = paused, 3 = game over
#define GAME_STATUS_SHIFT   (0)
#define GAME_PLAYING_LEVEL  (0b11111100) // Between 1 and 63
#define GAME_PLAYING_LEVEL_SHIFT (2)

// Bit Definitions for the current_game_state2
#define GAME_NUM_PLAYERS  (0b00000001)  // 0 = one player, 1 = two player
#define GAME_POISON_FLAG  (0b00000010)  // 0 = no, 1 = yes
#define GAME_POISON_SHIFT (1)
#define GAME_SPEED        (0b11111100) // Between 0 and 60
#define GAME_SPEED_SHIFT  (2)

// Bit Definitions for the current_game_state3
#define GAME_CAUSE_OF_DEATH    (0b00001111) // snake_status_t
#define GAME_CAUSE_OF_DEATH_SHIFT (0)
#define GAME_GRID_SIZE         (0b00110000) // 0 = 16x16, 1 = 32x16, 2 = 32x32
#define GAME_GRID_SIZE_SHIFT   (4)

// 32-Bit Definitions for the date/time
// 31    26   22    17    12     6      0
// +------+----+-----+-----+------+------+
// |YYYYYY|MMMM|DDDDD|HHHHH|MMMMMM|SSSSSS|
// +------+----+-----+-----+------+------+
#define YEAR_MASK   (0b00111111 << 26)
#define YEAR_SHIFT  (26)
#define MONTH_MASK  (0b00001111 << 22)
#define MONTH_SHIFT (22)
#define DAY_MASK    (0b00011111 << 17)
#define DAY_SHIFT   (17)
#define HOUR_MASK   (0b00011111 << 12)
#define HOUR_SHIFT  (12)
#define MINUTE_MASK (0b00111111 << 6)
#define MINUTE_SHIFT (6)
#define SECOND_MASK (0b00111111)
#define SECOND_SHIFT (0)

// 32-Bit Definitions for the command
// 31  27   23      16        8        0
// +-----+----+-------+--------+--------+
// |00000| CMD|RESERVE|  PARAM2|  PARAM1|
// +-----+----+-------+--------+--------+

#define I2C_CMD_SET_SPEED       (0b0001 << 23)
#define I2C_CMD_SET_LEVEL       (0b0010 << 23)
#define I2C_CMD_SET_POISON      (0b0011 << 23)
#define I2C_CMD_PREPARE_GAME    (0b0100 << 23)
#define I2C_CMD_START_GAME      (0b0101 << 23)
#define I2C_CMD_PAUSE_GAME      (0b0110 << 23)
#define I2C_CMD_END_GAME        (0b0111 << 23)
#define I2C_CMD_RANDOM_SEED     (0b1000 << 23)
#define PARAM1_MASK             (0b11111111)
#define PARAM1_SHIFT            (0)
#define PARAM2_MASK             (0b11111111 << 8)
#define PARAM2_SHIFT            (8)

typedef struct {                    // Register Map
    uint8_t console_info;           // 0x00
    uint8_t current_game_state;     // 0x01
    uint8_t current_game_state2;    // 0x02
    uint8_t current_game_state3;    // 0x03
    uint16_t current_score1;        // 0x04
    uint16_t current_score2;        // 0x06
    uint16_t number_apples1;        // 0x08
    uint16_t number_apples2;        // 0x0A
    uint16_t high_score;            // 0x0C
    uint16_t playing_time;          // 0x0E
    uint16_t num_apples_easy;       // 0x10
    uint16_t num_apples_medium;     // 0x12
    uint16_t num_apples_hard;       // 0x14
    uint16_t num_apples_insane;     // 0x16
    uint16_t high_score_easy;       // 0x18
    uint16_t high_score_medium;     // 0x1A
    uint16_t high_score_hard;       // 0x1C
    uint16_t high_score_insane;     // 0x1E
    char initials_easy[3];          // 0x20
    char initials_medium[3];        // 0x23
    char initials_hard[3];          // 0x26
    char initials_insane[3];        // 0x29
    uint32_t date_time;             // 0x2C
    uint32_t command;               // 0x30
    uint32_t random_seed;           // 0x34
} i2c_scoreboard_t;

typedef enum mode {
    TERMINAL_CONSOLE_MODE, PC_CONSOLE_MODE, SCOREBOARD_MODE, NUM_MODES
} mode_t;

typedef enum {
    NO_COLLISION, SNAKE_BODY_COLLISION, FOOD_COLLISION, POISON_FOOD_COLLISION, WALL_COLLISION, SNAKE_SELF_COLLISION
} snake_collision_t;

typedef enum {
    GRID_SIZE_16X16, GRID_SIZE_32X16, GRID_SIZE_32X32, NUM_GRID_SIZE_OPTIONS
} grid_size_options_t;

typedef struct score {
    uint8_t console_id;
    grid_size_options_t grid_size;
    uint8_t clock_sync;
    uint8_t game_status;
    options_difficulty_t game_difficulty;
    snake_collision_t cause_of_death;
    uint8_t game_speed;
    uint8_t is_connected;
    uint16_t score1;
    uint16_t score2;
    uint16_t apples1;
    uint16_t apples2;
    uint16_t playing_time;
    uint8_t level;
    uint8_t playing_mode;
    uint8_t with_poison;
} score_t;

typedef struct stats {
    uint16_t num_apples_easy;
    uint16_t num_apples_medium;
    uint16_t num_apples_hard;
    uint16_t num_apples_insane;
    uint16_t high_score_easy;
    uint16_t high_score_medium;
    uint16_t high_score_hard;
    uint16_t high_score_insane;
    char initials_easy[4];
    char initials_medium[4];
    char initials_hard[4];
    char initials_insane[4];
} stats_t;

typedef struct scoreboard {
    uint8_t num_consoles;
    mode_t mode;
    uint8_t polling_mode;
    uint8_t demo_mode;
    uint8_t is_demo_mode_initialized;
    score_t scores[MAX_NUM_CONSOLES];
    stats_t stats[MAX_NUM_CONSOLES];
    uint32_t random_seed;
} scoreboard_t;


void scoreboard_init();
void scoreboard_start();

#endif /* INC_SCOREBOARD_H_ */
