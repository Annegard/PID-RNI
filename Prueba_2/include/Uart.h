#ifndef UART_H_
#define UART_H_
/*==================[ Inclusiones ]============================================*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "driver/uart.h"
#include "sdkconfig.h"
#include "freertos/queue.h" //incluyo la libreria para usar cola

//Fuentes:
//https://www.sigmaelectronica.net/trama-gps/
//https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32/tree/master/src
//https://github.com/Tinyu-Zhao/TinyGPSPlus-ESP32/tree/master/src

/*==================[ Definiciones ]===================================*/

void config_Uart(void);

#endif