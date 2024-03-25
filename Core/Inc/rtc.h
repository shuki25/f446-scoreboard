/*
 * rtc.h
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#ifndef INC_RTC_H_
#define INC_RTC_H_

#include "main.h"
#include <stdint.h>

void RTC_sync_set_date(uint16_t year, uint16_t month, uint16_t day);
void RTC_sync_set_time(uint16_t hour, uint16_t minute, uint16_t second);
void RTC_get_date(uint16_t *year, uint16_t *month, uint16_t *day);
void RTC_get_time(uint16_t *hour, uint16_t *minute, uint16_t *second);

#endif /* INC_RTC_H_ */
