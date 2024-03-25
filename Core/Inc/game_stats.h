/*
 * game_stats.h
 *
 *  Created on: Jan 26, 2024
 *      Author: Joshua Butler, MD, MHI
 */

#ifndef INC_GAME_STATS_H_
#define INC_GAME_STATS_H_

#include <stdint.h>
#include "cmsis_os.h"

/*
 * Data structure for storing game statistics
 *
 * difficulty: game difficulty
 * poison: poison status
 * high_score: high score
 * level: current level
 * num_apples_eaten: number of apples eaten
 * length_played: length of time played
 * sDate: date of game
 * player_name: player name
 * times_played: number of games played at this difficulty
 *
 * Total size of struct must be less than 32 bytes as the EEPROM page size is 32 bytes.
 *
 * Current stuct sizes:
 * game_stats_t: 25 bytes
 * global_stats_t: 16 bytes
 *
 * If the struct size is changed, the EEPROM must be
 * reinitialized and the data will be reset to initial values.
 *
 */

typedef enum {
    SINGLE_PLAYER, TWO_PLAYERS_SNAKE, TWO_PLAYERS_FOOD
} snake_game_mode_t;

typedef enum {
    ONE_PLAYER, TWO_PLAYERS
} options_num_players_t;

typedef enum {
    EASY, MEDIUM, HARD, INSANE, NUM_DIFFICULTIES
} options_difficulty_t;

typedef enum {
    NO_POISON, POISON
} options_poison_t;

typedef struct {
    options_num_players_t num_players;
    options_difficulty_t difficulty;
    options_poison_t poison;
    uint8_t num_poisons;
} game_options_t;

typedef struct game_stats {
    options_difficulty_t difficulty;
    options_poison_t poison;
    uint16_t high_score;
    uint8_t level;
    uint16_t num_apples_eaten;
    uint32_t length_played;
    RTC_DateTypeDef sDate;
    char player_name[4];
} game_stats_t;

typedef struct global_stats {
    options_difficulty_t difficulty;
    uint16_t death_by_poison;
    uint16_t death_by_wall;
    uint16_t death_by_self;
    uint16_t total_games_played;
    uint16_t total_apples_eaten;
    uint32_t total_time_played;
} global_stats_t;

#endif /* INC_GAME_STATS_H_ */
