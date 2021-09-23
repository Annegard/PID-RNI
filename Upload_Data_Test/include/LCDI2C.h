#ifndef LCDI2C_H_
#define LCDI2C_H_
/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"



/*==================[Prototipos de funciones]======================*/
//static void I2C_init(void);
void LCDI2C_init(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print( char * str );

#endif