/*
 * rtc.c
 *
 *  Created on: Mar 14, 2024
 *      Author: josh
 */

#include "rtc.h"
#include "cmsis_os.h"
#include <string.h>

extern RTC_HandleTypeDef hrtc;

void RTC_sync_set_date(uint16_t year, uint16_t month, uint16_t day) {
    RTC_DateTypeDef sDate;
    memset(&sDate, 0, sizeof(sDate));

    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    sDate.Date = day;
    sDate.Month = month;
    sDate.Year = year - 2000;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        Error_Handler();
    }
}

void RTC_sync_set_time(uint16_t hour, uint16_t minute, uint16_t second)
{
    RTC_TimeTypeDef sTime;
    memset(&sTime, 0, sizeof(sTime));
    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        Error_Handler();
    }
//    __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
//    RTC_EnterInitMode(&hrtc);
//    RTC->CR &= ~RTC_CR_FMT;
//    RTC_ExitInitMode(&hrtc);
//    __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

}

void RTC_get_date(uint16_t *year, uint16_t *month, uint16_t *day) {
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    *year = sDate.Year + 2000;
    *month = sDate.Month;
    *day = sDate.Date;
}

void RTC_get_time(uint16_t *hour, uint16_t *minute, uint16_t *second) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    *hour = sTime.Hours;
    *minute = sTime.Minutes;
    *second = sTime.Seconds;
}
