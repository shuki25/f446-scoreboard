/*
 * i2c_master.h
 *
 *  Created on: Mar 18, 2024
 *      Author: josh
 */

#ifndef INC_I2C_MASTER_H_
#define INC_I2C_MASTER_H_

#include "scoreboard.h"

#define I2C_TIMEOUT (15)
#define I2C_SLAVE_START_ADDR (0x10)
#define I2C_BUFFER_SIZE (0x3F)
#define REGISTERS_SIZE (0x34)

typedef struct {
    uint16_t i2c_addr;
    uint8_t device_id;
    uint8_t is_active;
} device_list_t;

void initialize_device_list(device_list_t list[]);
void initialize_scoreboard_data(i2c_scoreboard_t *data);
void struct2register(i2c_scoreboard_t *data);
void register2struct(uint8_t reg[], i2c_scoreboard_t *data);
HAL_StatusTypeDef get_console_data(I2C_HandleTypeDef *hi2c, uint16_t i2c_addr, uint8_t reg_addr,
        uint8_t *data, uint8_t len);
HAL_StatusTypeDef fetch_scoreboard_data(I2C_HandleTypeDef *hi2c, device_list_t *device, uint8_t scoreboard_data[]);
HAL_StatusTypeDef i2c_master_scan(I2C_HandleTypeDef *hi2c, device_list_t device[]);
HAL_StatusTypeDef i2c_send_command(I2C_HandleTypeDef *hi2c, device_list_t device[], uint32_t command);
#endif /* INC_I2C_MASTER_H_ */
