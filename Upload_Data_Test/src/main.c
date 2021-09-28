//==================== Inclusiones ==========================
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "soc/soc.h" //disable brownout detector
#include "soc/rtc_cntl_reg.h" //disable brownout detector (deteccion de apagon)
#include "soc/rtc_wdt.h"

#include "../include/I2C.h"
#include "../include/RTC.h"
#include "../include/Uart.h"
#include "../include/LCDI2C.h"
#include "../include/WIFI_Control.h"

//=========================== Definiciones ================================

//Posicion del registro interno del RTC donde se encuentra cada uno de los siguiente valores (VER DATASHEET DS3231)
#define SEGUNDOS 0X00 
#define MINUTOS 0X01
#define HORA 0X02
#define DIA_SEMANA 0X03  //Establece el dia de la semana del 1 al 7
#define DIA_MES 0X04     //Establece el numero del dia en el mes
#define MES 0X05
#define ANIO 0X06
//pines de I2C
#define I2C_MASTER_SCL_IO 5            /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 4            /*!< gpio number for I2C master data  */

static void tareaHttp(void *pvParameters);

//================== Variables y constantes =======================
const char *TAG = "Ejemplo_thingspeak_client";
const char *REQUEST = "GET /update?api_key=KPC7Y114VMMZOF59&field1=%i\r\n"; //Solicitud para escribir el campo 1
int valor = 0;

//================== Función principal =======================
void app_main(void)
{
    i2c_master_init(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);     //Inicializo el i2c
    LCDI2C_init();  //Inicializo el LCD
    //config_Uart();  //Configuro el uart para recibir dato y cambiar los valores de fecha y hora
    iniciarWifi();

    esp_err_t ret = xTaskCreate(&tareaHttp, "tareaHttp", 4096, NULL, 5, NULL);
    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "Error al crear tarea");
    
   //Bucle infinito 
    while(1) {
        /*
        //obtengo datos desde el RTC
	    uint8_t Segundos;
        uint8_t Minutos;
        uint8_t Hora;
        uint8_t Dia;
        uint8_t Mes;
        uint8_t Anio;
        RTC_read(SEGUNDOS, &Segundos);
        RTC_read(MINUTOS, &Minutos);
        RTC_read(HORA, &Hora);
        RTC_read(DIA_MES, &Dia);
        RTC_read(MES, &Mes);
        RTC_read(ANIO, &Anio);

        //Imprimo los resultados en el monitor serial
		printf("%02x/%02x/%02x - %02x:%02x:%02x\n", Dia, Mes, Anio, Hora, Minutos, Segundos);

        //Imprimo los resultados en el display lcd 16x2 i2c
        char fila1[16]; //arreglo de caracteres a mostrar en fila 1
        char fila2[16]; //arreglo de caracteres a mostrar en fila 2
        sprintf( fila1, "    %02x/%02x/%02x", Dia, Mes, Anio); //Escribo arreglo de fila 1
        sprintf( fila2, "    %02x:%02x:%02x", Hora, Minutos, Segundos); //Escribo arreglo de fila 2

        lcd_gotoxy(1,1);  //Posiciono el cursor en el caracter 1-1
        lcd_print(fila1); //Imprimo fila 1 en display
        lcd_gotoxy(1,2); //Posiciono el cursor en el caracter 1-2
        lcd_print(fila2); //Imprimo fila 1 en display
        vTaskDelay(1000 / portTICK_PERIOD_MS); //Espero 1000 milisegundo*/
        }
    }
}

//================== Funciones =======================
static void tareaHttp(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s;

    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res); //Usa DNS para buscar informacion de la dirección IP

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "Error de busqueda de DNS, err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS); //espera 1 segundo
            continue; //vuelve al inicio del while
        }

        //Filtra y muestra en el monitor  la dirección IP
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "La búsqueda de DNS se realizó correctamente. IP=%s", inet_ntoa(*addr));

        //Crea un socket s
        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "No se pudo asignar el socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, ".Socket asignado");

        //Se conecta al socket
        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "La conexión del socket falló, errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Socket conectado.");
        freeaddrinfo(res); //libera la memoria

        valor++; //Variable que se usa en este ejemplo para enviar un dato
        char datos [strlen(REQUEST)+10];
        sprintf(datos,REQUEST,valor); //concatena el string que permite realizar la solicitud con la variable valor
        err = write(s, datos, strlen(datos)); //envia la cadena datos de longitud strlen(datos) al socket
        if (err < 0) {  //si es menor a 0 significa que hubo un error
            ESP_LOGE(TAG, "Error al enviar al socket");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "Envío al socket correcto.");

        close(s);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Reiniciando conexión!");
    }
}