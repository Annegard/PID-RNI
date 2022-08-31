#ifndef I2C_H_
#define I2C_H_

/*==========================================================================================
    Descripcion:
    Libreria de protocolo I2C para configurar al microcontrolador como dispositivo Maestro

    Los pines de SDA y SCL se dictan en el Header (archivo .h) de la libreria como definiciones
    
    las funciones principales son:
    _Iniciar el periferico I2C del microcontrolador.
    _Configurarlo para entradas Pull-UP.
    _Definir los pines a utilizar.
    _Leer y escribir dispositivos esclavos.
==========================================================================================*/

/*==================[ Inclusiones ]============================================*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include <stdlib.h>

//=========================== Definiciones ================================

#define DATA_LENGTH 512                     /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128                  /*!< Data length for r/w test, [0,DATA_LENGTH] */
#define DELAY_TIME_BETWEEN_ITEMS_MS 1000    /*!< delay time between different test items */

#define I2C_MASTER_SCL_IO 19                /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 18                /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1            /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 1000000          /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */

#define WRITE_BIT I2C_MASTER_WRITE          /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ            /*!< I2C master read */
#define ACK_CHECK_EN 0x1                    /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                   /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                         /*!< I2C ack value */
#define NACK_VAL 0x1                        /*!< I2C nack value */

/*==================[Prototipos de funciones]======================*/
void I2C_init();
uint8_t Send_i2c(uint8_t dato, int8_t direccion);
uint8_t Send_array_i2c(uint8_t arreglo[], uint8_t num_caracteres, int8_t direccion);
void Read_i2c(uint8_t *variable, int8_t direccion);

#endif