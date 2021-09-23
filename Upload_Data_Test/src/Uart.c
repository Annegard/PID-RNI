#include "../include/Uart.h"
#include "../include/RTC.h"

//=========================== Definiciones ================================
#define PIN_TX (UART_PIN_NO_CHANGE)
#define PIN_RX (UART_PIN_NO_CHANGE)
#define PIN_RTS (UART_PIN_NO_CHANGE)
#define PIN_CTS (UART_PIN_NO_CHANGE)
#define UART_PORT_NUM     (UART_NUM_0)
#define UART_BAUD_RATE     115200
#define BUF_SIZE (1024)
#define PROCESADORA 0
#define PROCESADORB 1


//========================== Prototipos ====================================
static void tareaEco(void *arg);


/*==================[Implementaciones]=================================*/


/*========================================================================
Funcion: config_Uart
Descripcion: Crea una tarea encargada de monitoriar el Uart y procesar el dato recibido
No retorna nada
========================================================================*/
void config_Uart(void)
{
    xTaskCreatePinnedToCore(tareaEco, "uart_tareaEco", configMINIMAL_STACK_SIZE*5, NULL, tskIDLE_PRIORITY+1, NULL, PROCESADORB);
}


/*========================================================================
Funcion: tareaEco
Descripcion: Funcion de la tarea
No retorna nada
========================================================================*/
static void tareaEco(void *arg)
{
    // Parámetros de configuración para la UART
    uart_config_t uartConfig = {
        .baud_rate = UART_BAUD_RATE,              
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // Configura la UART con los parámetros anteriores    
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uartConfig));

    // Configura los pines de la UART (TX: IO17 (UART2 default), RX: IO16 (UART2 default), RTS: no usado, CTS: no usado)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_TX, PIN_RX, PIN_RTS, PIN_CTS));

    //Instala el controlador UART y establece la UART en la configuración predeterminada.
    //El controlador UART ISR se adjuntará al mismo núcleo de CPU en el que se ejecuta esta función.
      
    ESP_ERROR_CHECK(uart_driver_install(
                        UART_PORT_NUM,     // Número de puerto
                        BUF_SIZE * 2,   // buffer rx 
                        0,              // buffer TX -- 0 no usa, la función de transmisión bloqueará la tarea hasta que se envién los datos
                        0,              // tamaño de la cola UART
                        NULL,           // handle de la cola UART
                        0               // flag para interrupciones
                    ));
    
    uint8_t data[BUF_SIZE]; // configura un buffer para los datos entrantes

    while (true) 
    {
        int len = uart_read_bytes(UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS); //lee desde la UART
        vTaskDelay(1000/portTICK_PERIOD_MS);
        
        if(len==19){     // Si recibo la cantidad de caracteres correctos
            if(data[2]=='/' && data[5]=='/' && data[8]==' ' && data[9]=='-' && data[10]==' ' && data[13]==':' && data[16]==':' ){ // Si la estructura es la correcta
               
                //Convierto la cadena de caracteres recibida a los valores correspondientes con los parametros a modificar 
                uint8_t dia=(data[0]-48)*10+(data[1]-48);
                uint8_t mes=(data[3]-48)*10+(data[4]-48);
                uint8_t anio=(data[6]-48)*10+(data[7]-48);
                uint8_t hora=(data[11]-48)*10+(data[12]-48);
                uint8_t minutos=(data[14]-48)*10+(data[15]-48);
                uint8_t segundos=(data[17]-48)*10+(data[18]-48);

                RTC_setTime(segundos, minutos, hora, 1, dia, mes, anio); //Establesco los valores recibidos como valores nuevos del RTC
                // second, minute, hour, dayOfWeek 1-7, dayOfMonth, month, year)
            }
        }

    }
}