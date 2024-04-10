/*
 * commands.c
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#include "commands.h"
#include "rtc.h"
#include "ui.h"

/*-----------------------------------------------------------------------------
 * List of valid console/PC commands
 *---------------------------------------------------------------------------*/

const char *valid_commands[] = { "", "@terminal", "@pc_console", "@scoreboard", "@set_date", "@set_time",
        "@get_date", "@get_time", "@devices", "@scores", "@poll", "@demo", "@stats", NULL };
uint8_t valid_num_params[] = { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 };
const char *snake_names[] =
        { "", "Ball Python", "Red-Tail Boa", "Black Rat Snake", "King Snake", "Corn Snake" };

command_t parse_command(uint8_t *command, uint8_t *token, uint8_t *parameter) {
    uint8_t num_returned;
    num_returned = sscanf((char*) command, "%s %s", token, (char*) parameter);
    uint8_t i = 0;
    while (valid_commands[i] != NULL) {
        if (strcmp((char*) token, valid_commands[i]) == 0) {
            if (valid_num_params[i] != num_returned - 1) {
                return INVALID_COMMAND;
            }
            return i;
        }
        i++;
    }
    return INVALID_COMMAND;
}

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

