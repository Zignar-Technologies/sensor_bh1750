#pragma once

#include <stdbool.h>

#define TICK_DELAY 10              //pdMS_TO_TICKS(10)
#define BH1750_CMD_START 0x23      /*!< Operation mode */
#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */
#define ACK_CHECK_EN 0x1           /*!< I2C master will check ack from slave*/
#define ACK_VAL 0x0                /*!< I2C ack value */
#define NACK_VAL 0x1               /*!< I2C nack value */

bool check_bh1750(void);
void init_bh1750(bool, uint32_t *arg);
char *get_bh1750(int a, bool raw);
void print_bh1750(void);
bool calibrate_bh1750(int a ,float* raws, float* instruments);
bool save_coefs_bh1750(int a);
bool load_coefs_bh1750(int a);
