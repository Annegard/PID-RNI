#ifndef Uart_H_
#define Uart_H_
/*==================[ Inclusiones ]============================================*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "driver/uart.h"
#include "sdkconfig.h"
#include "freertos/queue.h" //incluyo la libreria para usar cola


/*==================[ Variables ]===================================*/



/*==================[ Definiciones ]===================================*/

void config_Uart(void);

//#include "../main/Uart.c"
#endif