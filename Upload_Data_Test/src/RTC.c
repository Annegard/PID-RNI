/*==================[ Inclusiones ]============================================*/
#include "../include/RTC.h"
#include "../include/I2C.h"

/*==================[Prototipos de funciones]======================*/
void RTC_read(uint8_t instruccion, uint8_t *variable);
void RTC_setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
                   dayOfMonth, uint8_t month, uint8_t year);

//=========================== Definiciones ================================
#define ESP_SLAVE_ADDR_RTC 0x68                /*!< ESP32 slave address, you can set any 7bit value */

/*========================================================================
Funcion: RTC_read
Descripcion: Lee la fecha y hora desde el RTC
Parametro de entrada:
       uint8_t instruccion: Indica que parametro se quiere leer 
                #define SEGUNDOS 0X00 
                #define MINUTOS 0X01
                #define HORA 0X02
                #define DIA_SEMANA 0X03  //Establece el dia de la semana del 1 al 7
                #define DIA_MES 0X04     //Establece el numero del dia en el mes
                #define MES 0X05
                #define ANIO 0X06
       uint8_t *variable: Puntero hacia la variable donde se quiere guardar el dato obtenido del RTC
No retorna nada
========================================================================*/
void RTC_read(uint8_t instruccion, uint8_t *variable){ 
    Send_i2c(instruccion, ESP_SLAVE_ADDR_RTC);  //Envia al i2c a la direccion del RTC la instruccion del
                                                //valor que se quiere obtener
    Read_i2c(variable, ESP_SLAVE_ADDR_RTC); //Se lee lo que retorna el RTC esclavo por el puerto I2C y se 
                                            //guarda el dato en la variable referida por puntero
}


/*========================================================================
Funcion: dec2bcd
Descripcion: Convierte numero decimal a bcd
Parametro de entrada:
       uint8_t num: Numero a convertir en decimal
Retorna: numero convertido a bcd
========================================================================*/
uint8_t dec2bcd(uint8_t num)
{
    uint8_t ones = 0;
    uint8_t tens = 0;
    uint8_t temp = 0;
    ones = num%10;
    temp = num/10;
    tens = temp<<4;
    return (tens + ones);
}


/*========================================================================
Funcion: RTC_setTime
Descripcion: Establece la fecha y hora del RTC
Parametro de entrada:
       uint8_t second: Valor de los segundos a establecer en el RTC en decimal
       uint8_t minute: Valor de los minutos a establecer en el RTC en decimal
       uint8_t hour: Valor de la hora a establecer en el RTC en decimal
       uint8_t dayOfWeek: Valor del dia de la semana (1-7) a establecer en el RTC en decimal
       uint8_t dayOfMonth: Valor del dia del mes a establecer en el RTC en decimal
       uint8_t month: Valor del numero del mes a establecer en el RTC en decimal
       uint8_t year: Valor del numero del año en formato xx a establecer en el RTC en decimal
No retorna nada
========================================================================*/
void RTC_setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
                   dayOfMonth, uint8_t month, uint8_t year){

    uint8_t arreglo_datos[7]; //Arreglo para almacenar todos los valores a establecer y luego enviar solo el arreglo
    arreglo_datos[0]=0x00; //Instruccion de donde se quiere empezar a modificar los datos, en este caso se modifica 
    //desde la posicion cero que es la posicoon de los segundos, Ver datasheet DS3231 Los datos se convierten de decimal 
    //a BCD para que queden los dos numeros decimales separado (Asi los establece el datasheet). Por ejemplo para la hora 
    //23 quedan los dos numeros separado en dos nibles de 4 bits
    arreglo_datos[1]=dec2bcd(second)&0b01111111;//Se convierte los segundos de decimal a BCD y se le coloca la "MASCARA" 
                                                //correspondiente con el datasheet DS3231 
    arreglo_datos[2]=dec2bcd(minute)&0b01111111;//Se realiza lo mismo   
    arreglo_datos[3]=dec2bcd(hour)&0b00111111;//Se realiza lo mismo  
    arreglo_datos[4]=dec2bcd(dayOfWeek)&0b00000111;//Se realiza lo mismo  
    arreglo_datos[5]=dec2bcd(dayOfMonth)&0b00111111;//Se realiza lo mismo  
    arreglo_datos[6]=dec2bcd(month)&0b00011111;//Se realiza lo mismo  
    arreglo_datos[7]=dec2bcd(year);//Se realiza lo mismo
    uint8_t ret = Send_array_i2c(arreglo_datos, 7, ESP_SLAVE_ADDR_RTC); //Envia el arreglo por i2c al RTC  

    if(ret==1){
        printf("Fecha y hora cambiada\n");
    }
}
