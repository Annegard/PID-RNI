#ifndef RTC_H_
#define RTC_H_
/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"


/*==================[Prototipos de funciones]======================*/
void RTC_read(uint8_t instruccion, uint8_t *variable);
void RTC_setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
                   dayOfMonth, uint8_t month, uint8_t year);
//Byte bcdToDec(Byte val);


//#include "../main/RTC.c"
#endif