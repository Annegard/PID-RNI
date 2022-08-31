#ifndef LCDI2C_H_
#define LCDI2C_H_

/*==========================================================================================
    Descripcion:
    Libreria para controlar display LCD LM016 con modulo multiplexor pcf8574 para comunicion
    I2C.

    El nombre del display es por defecto 0x27 y se configuran las celdas como matriz de 5x7 matriz,
    modo 4 bits.

    Internamente el dato o comando a enviar se divide en dos partes mediante enmascaramiento y de
    los 8 bits los primeros 4 solo son informacion mientras que los ultimos 4 especifican el tipo
    de informacion, habilitacion, luz Buckground, etc.
==========================================================================================*/

/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"

//=========================== Definiciones ================================
#define ESP_SLAVE_ADDR_LCD 0x27                /*!< ESP32 slave address, you can set any 7bit value */

/*==================[Prototipos de funciones]======================*/
void LCDI2C_init(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print( char * str );
void lcdCommand(unsigned char cmnd);
void Print_Float_LCD(float Flotante);
void lcdData(unsigned char data);

#endif