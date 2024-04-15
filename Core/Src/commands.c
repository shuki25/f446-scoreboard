/*
 * commands.c
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#include "commands.h"
#include "rtc.h"
#include "ui.h"
#include <ctype.h>

/*-----------------------------------------------------------------------------
 * List of valid console/PC commands
 *---------------------------------------------------------------------------*/
// Must align with command_t
const char *valid_commands[] = { "", "", "@terminal", "@pc_console", "@scoreboard", "@set_date", "@set_time",
        "@get_date", "@get_time", "@devices", "@scores", "@poll", "@demo", "@stats", "@set_speed", "@set_level",
        "@prepare_game", "@start_game", "@end_game", "@pause_game", NULL };
uint8_t valid_num_params[] = { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 2, 1, 0, 0, 0 };
const char *snake_names[] =
        { "", "Ball Python", "Red-Tail Boa", "Black Rat Snake", "King Snake", "Corn Snake" };


/*-----------------------------------------------------------------------------
 * Function: trim_whitespace
 *
 * This function will trim whitespace from a string
 *
 * Parameters: char *dst - destination string
 *            char *src - source string
 * Return: None
 *
 *--------------------------------------------------------------------------- */
void trim_whitespace(char *dst, char *src) {
    uint8_t i = 0;
    uint8_t j = strlen(src);
    while (isspace((unsigned char)src[i])) {
        i++;
    }
    while (isspace((unsigned char)src[j])) {
        j--;
    }
    strncpy(dst, src + i, j - i + 1);
    dst[j] = '\0';
}

/*-----------------------------------------------------------------------------
 * Function: num_parameters
 *
 * This function will count the number of parameters in a string
 *
 * Parameters: char *parameters - string of parameters
 * Return: uint8_t - number of parameters
 *---------------------------------------------------------------------------*/
uint8_t num_parameters(char *parameters) {
    uint8_t count = 1;
    uint8_t str_len = strlen(parameters);
    if (str_len == 0) {
        return 0;
    }

    for (int i = 0; i < str_len; i++) {
        if (isspace((unsigned char)parameters[i])) {
            count++;
        }
    }
    return count;
}


uint32_t parse_i2c_command(command_t cmd_token, uint8_t *parameter) {
    uint32_t data = 0;
    uint32_t param1 = 0;
    uint32_t param2 = 0;
    switch (cmd_token) {
        case CMD_SET_SPEED: // param1 = speed
            sscanf((char *)parameter, "%ld", &param1);
            if (param1 > 0 && param1 <= 100) {
                data = param1 & PARAM1_MASK;
                data |= I2C_CMD_SET_SPEED;
            }
            break;
        case CMD_SET_LEVEL: // param1 = level
            sscanf((char *)parameter, "%ld", &param1);
            if (param1 > 0 && param1 <= 4) {
                data = param1 & PARAM1_MASK;
                data |= I2C_CMD_SET_LEVEL;
            }
            break;
        case CMD_PREPARE_GAME: // param1 = level, param2 = with_poison
            sscanf((char *)parameter, "%ld %ld", &param1, &param2);
            if (param1 >= 0 && param1 < 4 && param2 >= 0 && param2 <= 1) {
                data = param1 & PARAM1_MASK;
                data |= ((param2 << 8) & PARAM2_MASK);
                data |= I2C_CMD_PREPARE_GAME;
            }
            break;
        case CMD_START_GAME: // param1 = speed
            sscanf((char *)parameter, "%ld", &param1);
            if (param1 <= 100) {
                data = param1 & PARAM1_MASK;
                data |= I2C_CMD_START_GAME;
            }
            break;
        case CMD_END_GAME: // no parameters
            data |= I2C_CMD_END_GAME;
            break;
        case CMD_PAUSE_GAME: // no parameters
            data |= I2C_CMD_PAUSE_GAME;
            break;
        default:
            return 0;
            break;
    }
    return data;
}

/*-----------------------------------------------------------------------------
 * Function: parse_command
 *
 * This function will parse a command string and return the command token
 *
 * Parameters: uint8_t *command - command string
 *             uint8_t *token - token to return
 *             uint8_t *parameter - parameter to return
 * Return: command_t - command token
 *---------------------------------------------------------------------------*/
command_t parse_command(uint8_t *command, uint8_t *token, uint8_t *parameter) {

    char tmp_parameter[256];

    memset(tmp_parameter, 0, sizeof(tmp_parameter));

    strcpy((char *)token, strtok((char *)command, " "));
    strcpy((char *)parameter, strtok(NULL, ""));
//    sscanf((char*) command, "%s %s", token, (char*) parameter);
    uint8_t i = 0;

    trim_whitespace(tmp_parameter, (char*) parameter);
    strcpy((char*) parameter, tmp_parameter);

    uint8_t num_params = num_parameters((char*) parameter);

    while (valid_commands[i] != NULL) {
        if (strcmp((char*) token, valid_commands[i]) == 0) {
            if (valid_num_params[i] != num_params) {
                return INVALID_PARAMETER_COUNT;
            }
            return i;
        }
        i++;
    }
    return INVALID_COMMAND;
}

/*-----------------------------------------------------------------------------
 * Function: parse_date
 *
 * This function will parse a date string and return the year, month, and day
 *
 * Parameters: uint8_t *date - date string
 *             uint16_t *year - year to return
 *             uint16_t *month - month to return
 *             uint16_t *day - day to return
 * Return: uint8_t - 1 if successful, 0 if failed
 *---------------------------------------------------------------------------*/
uint8_t parse_date(uint8_t *date, uint16_t *year, uint16_t *month, uint16_t *day) {
    uint8_t num_returned;
    int y, m, d;

    num_returned = sscanf((char*) date, "%d-%d-%d", &y, &m, &d);
    if (num_returned < 3) {
        return 0;
    } else {
        *year = y;
        *month = m;
        *day = d;
        return 1;
    }
}

/*-----------------------------------------------------------------------------
 * Function: parse_time
 *
 * This function will parse a time string and return the hour, minute, and second
 *
 * Parameters: uint8_t *time - time string
 *             uint16_t *hour - hour to return
 *             uint16_t *minute - minute to return
 *             uint16_t *second - second to return
 * Return: uint8_t - 1 if successful, 0 if failed
 *---------------------------------------------------------------------------*/
uint8_t parse_time(uint8_t *time, uint16_t *hour, uint16_t *minute, uint16_t *second) {
    uint8_t num_returned;
    int h, m, s;

    num_returned = sscanf((char*) time, "%d:%d:%d", &h, &m, &s);
    if (num_returned < 3) {
        return 0;
    } else {
        *hour = h;
        *minute = m;
        *second = s;
        return 1;
    }
}

/*-----------------------------------------------------------------------------
 * Function: execute_command
 *
 * This function will execute a command
 *
 * Parameters: scoreboard_t *scoreboard - pointer to the scoreboard
 *             command_t command - command token
 *             uint8_t *parameter - parameter
 * Return: cmd_status_t - status of the command
 *---------------------------------------------------------------------------*/
cmd_status_t execute_command(scoreboard_t *scoreboard, command_t command, uint8_t *parameter) {
    uint16_t year, month, day, hour, minute, second;
    uint8_t num_console;
    char output_buffer[256];
    memset(output_buffer, 0, sizeof(output_buffer));

    switch (command) {
        case CMD_TERMINAL_MODE:
            scoreboard->mode = TERMINAL_CONSOLE_MODE;
            print_terminal(scoreboard, "\r\nTerminal mode enabled\r\n");
            break;
        case CMD_PC_MODE:
            scoreboard->mode = PC_CONSOLE_MODE;
            print_pc_console(scoreboard, "OK\n");
            break;
        case CMD_SCOREBOARD_MODE:
            scoreboard->mode = SCOREBOARD_MODE;
            print_scoreboard(scoreboard, "{'mode': 'scoreboard', 'status': 1}\n");
            break;
        case CMD_SET_DATE:
            if (parse_date(parameter, &year, &month, &day)) {
                RTC_sync_set_date(year, month, day);
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nDate set to %04d-%02d-%02d\r\n", year, month, day);
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "OK\t%04d-%02d-%02d\n", year, month, day);
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'date': '%04d-%02d-%02d', 'status': 1}\n", year, month, day);
                    print_scoreboard(scoreboard, output_buffer);
                }
            } else {
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nInvalid date\n");
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "ERR\tInvalid date\n");
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'error': 'Invalid date', 'status': 0}\n");
                    print_scoreboard(scoreboard, output_buffer);
                }
                return INVALID_COMMAND;
            }

            break;
        case CMD_SET_TIME:
            if (parse_time(parameter, &hour, &minute, &second)) {
                RTC_sync_set_time(hour, minute, second);
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nTime set to %02d:%02d:%02d\r\n", hour, minute, second);
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "OK\t%02d:%02d:%02d\n", hour, minute, second);
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'time': '%02d:%02d:%02d', 'status': 1}\n", hour, minute, second);
                    print_scoreboard(scoreboard, output_buffer);
                }
            } else {
                return INVALID_COMMAND;
            }
            break;
        case CMD_GET_DATE:
            RTC_get_time(&hour, &minute, &second);
            RTC_get_date(&year, &month, &day);
            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                sprintf(output_buffer, "\r\nCurrent date: %04d-%02d-%02d\r\n", year, month, day);
                print_terminal(scoreboard, output_buffer);
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                sprintf(output_buffer, "OK\t%04d-%02d-%02d\n", year, month, day);
                print_pc_console(scoreboard, output_buffer);
            } else {
                sprintf(output_buffer, "{'date': '%04d-%02d-%02d', 'status': 1}\n", year, month, day);
                print_scoreboard(scoreboard, output_buffer);
            }
            break;
        case CMD_GET_TIME:
            RTC_get_time(&hour, &minute, &second);
            RTC_get_date(&year, &month, &day);
            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                sprintf(output_buffer, "\r\nCurrent time: %02d:%02d:%02d\r\n", hour, minute, second);
                print_terminal(scoreboard, output_buffer);
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                sprintf(output_buffer, "OK\t%02d:%02d:%02d\n", hour, minute, second);
                print_pc_console(scoreboard, output_buffer);
            } else {
                sprintf(output_buffer, "{'time': '%02d:%02d:%02d', 'status': 1}\n", hour, minute, second);
                print_scoreboard(scoreboard, output_buffer);
            }
            break;
        case CMD_LIST_DEVICES:
            num_console = 0;
            for (int i = 0; i < scoreboard->num_consoles; i++) {
                if (scoreboard->scores[i].is_connected)
                    num_console++;
            }

            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                sprintf(output_buffer, "\r\nGaming Consoles:");
                print_terminal(scoreboard, output_buffer);
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer, " %s", snake_names[scoreboard->scores[i].console_id]);
                        print_terminal(scoreboard, output_buffer);
                    }
                }
                print_terminal(scoreboard, "\r\n");
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                sprintf(output_buffer, "OK\t%d", num_console);
                print_pc_console(scoreboard, output_buffer);
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer, "\t%s", snake_names[scoreboard->scores[i].console_id]);
                        print_pc_console(scoreboard, output_buffer);
                    }
                }
                print_pc_console(scoreboard, "\n");
            } else {
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (!scoreboard->scores[i].is_connected) {
                        continue;
                    }
                    if (i == 0) {
                        sprintf(output_buffer, "{\"num_devices\": %d, \"devices\":[", num_console);
                        print_scoreboard(scoreboard, output_buffer);
                    } else {
                        print_scoreboard(scoreboard, ",");
                    }
                    sprintf(output_buffer, "{\"console_id\": %d, \"snake_name\": \"%s\"}",
                            scoreboard->scores[i].console_id, snake_names[scoreboard->scores[i].console_id]);
                    print_scoreboard(scoreboard, output_buffer);
                }
                print_scoreboard(scoreboard, "]}\n");
            }
            break;
        case CMD_LIST_SCORES:
            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                sprintf(output_buffer, "\r\nScores: %d\r\n", scoreboard->num_consoles);
                print_terminal(scoreboard, output_buffer);
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer,
                                "Console %d: Score1: %d, Score2: %d, Apples1: %d, Apples2: %d, Level: %d, Poison: %d, Mode: %d, Status: %d\r\n",
                                i, scoreboard->scores[i].score1, scoreboard->scores[i].score2,
                                scoreboard->scores[i].apples1, scoreboard->scores[i].apples2,
                                scoreboard->scores[i].level, scoreboard->scores[i].with_poison,
                                scoreboard->scores[i].playing_mode, scoreboard->scores[i].game_status);
                        print_terminal(scoreboard, output_buffer);
                    }
                }
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                num_console = 0;
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected)
                        num_console++;
                }
                sprintf(output_buffer, "OK\t%d\n", num_console);
                print_pc_console(scoreboard, output_buffer);
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer, "CONSOLE %d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                                scoreboard->scores[i].console_id, scoreboard->scores[i].score1,
                                scoreboard->scores[i].score2, scoreboard->scores[i].apples1,
                                scoreboard->scores[i].apples2, scoreboard->scores[i].level,
                                scoreboard->scores[i].with_poison, scoreboard->scores[i].playing_mode,
                                scoreboard->scores[i].game_status, scoreboard->scores[i].playing_time);
                        print_pc_console(scoreboard, output_buffer);
                    }
                }
            } else {
                uint8_t first = 1;
                uint8_t has_scores = 0;
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (!scoreboard->scores[i].is_connected) {
                        continue;
                    }
                    has_scores = 1;
                    if (first) {
                        print_scoreboard(scoreboard, "\r\n{\"consoles\": {\"console\":[");
                        first = 0;
                    } else {
                        print_scoreboard(scoreboard, ",");
                    }

                    sprintf(output_buffer,
                            "{\"console_id\":%d, \"grid_size\":%d, \"clock_sync\": %d, \"game_status\": %d, \"game_difficulty\": %d, ",
                            scoreboard->scores[i].console_id, scoreboard->scores[i].grid_size,
                            scoreboard->scores[i].clock_sync, scoreboard->scores[i].game_status,
                            scoreboard->scores[i].game_difficulty);
                    print_scoreboard(scoreboard, output_buffer);
                    sprintf(output_buffer,
                            "\"cause_of_death\": %d, \"game_speed\": %d, \"is_connected\": %d, \"score1\": %d, \"score2\": %d, ",
                            scoreboard->scores[i].cause_of_death, scoreboard->scores[i].game_speed,
                            scoreboard->scores[i].is_connected, scoreboard->scores[i].score1,
                            scoreboard->scores[i].score2);
                    print_scoreboard(scoreboard, output_buffer);
                    sprintf(output_buffer,
                            "\"apples1\": %d, \"apples2\": %d, \"level\": %d, \"playing_mode\": %d, \"with_poison\": %d, ",
                            scoreboard->scores[i].apples1, scoreboard->scores[i].apples2,
                            scoreboard->scores[i].level, scoreboard->scores[i].playing_mode,
                            scoreboard->scores[i].with_poison);
                    print_scoreboard(scoreboard, output_buffer);
                    sprintf(output_buffer, "\"playing_time\": %d}", scoreboard->scores[i].playing_time);
                    print_scoreboard(scoreboard, output_buffer);
                }
                if (has_scores) {
                    print_scoreboard(scoreboard, "]}}\n");
                } else {
                    print_scoreboard(scoreboard, "{'consoles': 'none', 'status': 1}\n");
                }
            }
            break;
        case CMD_POLLING_MODE:
            if (strcmp((char*) parameter, "on") == 0) {
                scoreboard->polling_mode = 1;
            } else if (strcmp((char*) parameter, "off") == 0) {
                scoreboard->polling_mode = 0;
            } else {
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nInvalid polling mode\n");
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "ERR\tInvalid polling mode\n");
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'error': 'Invalid polling mode', 'status': 0}\n");
                    print_scoreboard(scoreboard, output_buffer);
                }
                return INVALID_COMMAND;
            }
            break;
        case CMD_DEMO_MODE:
            if (strcmp((char*) parameter, "on") == 0) {
                scoreboard->demo_mode = 1;
            } else if (strcmp((char*) parameter, "off") == 0) {
                scoreboard->demo_mode = 0;
            } else if (strcmp((char*) parameter, "status") == 0) {
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nDemo mode: %s\r\n", scoreboard->demo_mode ? "on" : "off");
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "OK\t%s\n", scoreboard->demo_mode ? "on" : "off");
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'demo_mode': '%s', 'status': 1}\n",
                            scoreboard->demo_mode ? "on" : "off");
                    print_scoreboard(scoreboard, output_buffer);
                }
            } else if (strcmp((char*) parameter, "reset") == 0) {
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    scoreboard->scores[i].score1 = 0;
                    scoreboard->scores[i].score2 = 0;
                    scoreboard->scores[i].apples1 = 0;
                    scoreboard->scores[i].apples2 = 0;
                    scoreboard->scores[i].level = 1;
                    scoreboard->scores[i].with_poison = 0;
                    scoreboard->scores[i].playing_mode = 0;
                    scoreboard->scores[i].game_status = 0;
                }
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nDemo mode reset\r\n");
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "OK\tDemo mode reset\n");
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'demo_mode': 'reset', 'status': 1}\n");
                    print_scoreboard(scoreboard, output_buffer);
                }
            } else {
                if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                    sprintf(output_buffer, "\r\nInvalid polling mode\n");
                    print_terminal(scoreboard, output_buffer);
                } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                    sprintf(output_buffer, "ERR\tInvalid polling mode\n");
                    print_pc_console(scoreboard, output_buffer);
                } else {
                    sprintf(output_buffer, "{'error': 'Invalid polling mode', 'status': 0}\n");
                    print_scoreboard(scoreboard, output_buffer);
                }
                return INVALID_COMMAND;
            }
            break;
        case CMD_STATS:
            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer, "\r\nStats for %s console:\r\n",
                                snake_names[scoreboard->scores[i].console_id]);
                        print_terminal(scoreboard, output_buffer);
                        sprintf(output_buffer,
                                "Apples\r\n=======================\r\nEasy: %d, Medium: %d\r\nHard: %d, Insane: %d\r\n",
                                scoreboard->stats[i].num_apples_easy, scoreboard->stats[i].num_apples_medium,
                                scoreboard->stats[i].num_apples_hard, scoreboard->stats[i].num_apples_insane);
                        print_terminal(scoreboard, output_buffer);
                        sprintf(output_buffer,
                                "High Scores\r\n=======================\r\nEasy: %d (%s), Medium: %d (%s)\r\nHard: %d (%s), Insane: %d (%s)\r\n",
                                scoreboard->stats[i].high_score_easy, scoreboard->stats[i].initials_easy,
                                scoreboard->stats[i].high_score_medium, scoreboard->stats[i].initials_medium,
                                scoreboard->stats[i].high_score_hard, scoreboard->stats[i].initials_hard,
                                scoreboard->stats[i].high_score_insane, scoreboard->stats[i].initials_insane);
                        print_terminal(scoreboard, output_buffer);
                    }
                }
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                num_console = 0;
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected)
                        num_console++;
                }
                sprintf(output_buffer, "OK\t%d\n", num_console);
                print_pc_console(scoreboard, output_buffer);
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (scoreboard->scores[i].is_connected) {
                        sprintf(output_buffer, "CONSOLE %d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
                                scoreboard->scores[i].console_id, scoreboard->stats[i].num_apples_easy,
                                scoreboard->stats[i].num_apples_medium, scoreboard->stats[i].num_apples_hard,
                                scoreboard->stats[i].num_apples_insane, scoreboard->stats[i].high_score_easy,
                                scoreboard->stats[i].high_score_medium, scoreboard->stats[i].high_score_hard,
                                scoreboard->stats[i].high_score_insane,
                                scoreboard->stats[i].high_score_insane);
                        print_pc_console(scoreboard, output_buffer);
                        sprintf(output_buffer, "\t%s\t%s\t%s\t%s\n", scoreboard->stats[i].initials_easy,
                                scoreboard->stats[i].initials_medium, scoreboard->stats[i].initials_hard,
                                scoreboard->stats[i].initials_insane);
                    }
                }
            } else {
                uint8_t first = 1;
                uint8_t has_stats = 0;
                for (int i = 0; i < scoreboard->num_consoles; i++) {
                    if (!scoreboard->scores[i].is_connected) {
                        continue;
                    }
                    has_stats = 1;
                    if (first) {
                        print_scoreboard(scoreboard, "\r\n{\"stats\": {\"console\":[");
                        first = 0;
                    } else {
                        print_scoreboard(scoreboard, ",");
                    }

                    sprintf(output_buffer,
                            "{\"console_id\":%d, \"num_apples_easy\":%d, \"num_apples_medium\": %d, \"num_apples_hard\": %d, \"num_apples_insane\": %d, ",
                            scoreboard->scores[i].console_id, scoreboard->stats[i].num_apples_easy,
                            scoreboard->stats[i].num_apples_medium, scoreboard->stats[i].num_apples_hard,
                            scoreboard->stats[i].num_apples_insane);
                    print_scoreboard(scoreboard, output_buffer);
                    sprintf(output_buffer,
                            "\"high_score_easy\": %d, \"high_score_medium\": %d, \"high_score_hard\": %d, \"high_score_insane\": %d, ",
                            scoreboard->stats[i].high_score_easy, scoreboard->stats[i].high_score_medium,
                            scoreboard->stats[i].high_score_hard, scoreboard->stats[i].high_score_insane);
                    print_scoreboard(scoreboard, output_buffer);
                    sprintf(output_buffer,
                            "\"initials_easy\": \"%s\", \"initials_medium\": \"%s\", \"initials_hard\": \"%s\", \"initials_insane\": \"%s\"}",
                            scoreboard->stats[i].initials_easy, scoreboard->stats[i].initials_medium,
                            scoreboard->stats[i].initials_hard, scoreboard->stats[i].initials_insane);
                    print_scoreboard(scoreboard, output_buffer);
                }
                if (has_stats) {
                    print_scoreboard(scoreboard, "]}}\n");
                } else {
                    print_scoreboard(scoreboard, "{'stats': 'none', 'status': 1}\n");
                }
            }
            break;
        default:
            if (scoreboard->mode == TERMINAL_CONSOLE_MODE) {
                sprintf(output_buffer, "\r\nInvalid command\n");
                print_terminal(scoreboard, output_buffer);
            } else if (scoreboard->mode == PC_CONSOLE_MODE) {
                sprintf(output_buffer, "ERR\tUnknow Command\n");
                print_pc_console(scoreboard, output_buffer);
            } else if (scoreboard->mode == SCOREBOARD_MODE) {
                sprintf(output_buffer, "{'error': 'Unknown command', 'status': 0}\n");
                print_scoreboard(scoreboard, output_buffer);
            }
            return INVALID_COMMAND;
            break;
    }
    return CMD_OK;
}

