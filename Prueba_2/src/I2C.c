/*==================[ Inclusiones ]============================================*/
#include "../include/I2C.h"

/*==================[Prototipos de funciones]======================*/
static esp_err_t i2c_master_init(void);

//=========================== Variables ================================
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //Inicializa el spinlock desbloqueado

/*==================[Implementaciones]=================================*/

/*========================================================================
Funcion: I2C_Init
Descripcion:    Funcion para ejecutar de manera externa a la libreria la funcion
                I2C_master_init(). Por alguna razon al definirla en el header la
                toma como no definida y no se puede llamar directamente por lo que
                se implementa la funcion I2C_Init().
Parametro de entrada:
      No recibe nada
Retorna:
        No retorna nada
========================================================================*/
void I2C_init(){
    ESP_ERROR_CHECK(i2c_master_init());
}

/*========================================================================
Funcion: Send_i2c
Descripcion: Funcion para enviar un dato por medio de i2c
Parametro de entrada:
       uint8_t dato: dato a enviar en tipo entero sin signo de 8 bit
       int8_t direccion: direccion del i2c esclavo al que se envia el dato
Retorna:
        1 si se pudo realizar el envio
        0 si fallo el envio
========================================================================*/
uint8_t Send_i2c(uint8_t dato, int8_t direccion){

    int ret; //Variable para saber si se realizo la comunicacion
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); //Crea una estructura para almacenar los siguientes valores
    i2c_master_start(cmd); //Inicia la comunicacion (Bit de start)
    i2c_master_write_byte(cmd, (direccion << 1) | WRITE_BIT, ACK_CHECK_EN); //Primero envia la direccion del esclavo al cual se desea comunicar y activa el chequeo de ack
    i2c_master_write(cmd, &dato, sizeof(dato), ACK_CHECK_EN); //Segundo se envia el dato y activa el chequeo de ack
    i2c_master_stop(cmd); //Detiene la comunicacion (Bit de stop)
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS); //Envia toda la estructura con todo lo anterior por los pines establecido de i2c del micro
    i2c_cmd_link_delete(cmd);//Elimina la estructura

    if (ret == ESP_OK) { //Si el retorno de la funcion de envio es correcto
        return 1; //La funcion retorna 1
    }else{ //Sino
        return 0; //La funcion retorna 0
        printf("Errro I2C : %s!\n", esp_err_to_name(ret));//Imprime el error por monitor serial
     }                     
}

/*========================================================================
Funcion: Send_array_i2c
Descripcion: Funcion para enviar varios datos comprendidos en un arreglo por medio de i2c
Parametro de entrada:
       uint8_t arreglo[]: arreglo que contiene los datos a enviar en tipo entero sin signo de 8 bit
       uint8_t num_caracteres: cantidad de caracteres o datos que tiene el arreglo
       int8_t direccion: direccion del i2c esclavo al que se envia el dato
Retorna:
        1 si se pudo realizar el envio
        0 si fallo el envio
========================================================================*/

uint8_t Send_array_i2c(uint8_t arreglo[], uint8_t num_caracteres, int8_t direccion){
 
    int ret;//Variable para saber si se realizo la comunicacion
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();//Crea una estructura para almacenar los siguientes valores
    i2c_master_start(cmd);//Inicia la comunicacion (Bit de start)
    i2c_master_write_byte(cmd, (direccion << 1) | WRITE_BIT, ACK_CHECK_EN); //Primero envia la direccion del esclavo al cual se desea comunicar y activa el chequeo de ack
    
    for(uint8_t i=0; i<=num_caracteres; i++) //Recorre el arreglo para enviar de a un dato a la vez
	{
        i2c_master_write(cmd, &arreglo[i], sizeof(arreglo[i]), ACK_CHECK_EN); //Se envian todos los datos del arreglo y activa el chequeo de ack en cada envio
	}
    i2c_master_stop(cmd); //Detiene la comunicacion (Bit de stop)
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);//Envia toda la estructura con todo lo anterior por los pines establecido de i2c del micro. Establece un tiempo limite para no clavar el micro si no se recibe nada
    i2c_cmd_link_delete(cmd); //Elimina la estructura

    if (ret == ESP_OK) { //Si el retorno de la funcion de envio es correcto
        return 1; //La funcion retorna 1
    }else{ //Sino
        return 0; //La funcion retorna 0
        printf("Error I2C : %s!\n", esp_err_to_name(ret)); //Imprime el error por monitor serial
     } 
                       
}


/*========================================================================
Funcion: Read_i2c
Descripcion: Lee el puerto I2C y guarda el dato en la variable enviada como puntero
Parametro de entrada:
       uint8_t *variable: puntero de la variable donde se quiere guardar el el dato recibido por i2c
       int8_t direccion: direccion del i2c esclavo al que se envia el dato
No retorna nada
========================================================================*/

void Read_i2c(uint8_t *variable, int8_t direccion){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); //Crea una estructura para almacenar los siguientes valores
    i2c_master_start(cmd); //Inicia la comunicacion (Bit de start)
    i2c_master_write_byte(cmd, (direccion << 1) | READ_BIT, ACK_CHECK_EN); //Primero envia la direccion del esclavo al cual se desea comunicar y activa el chequeo de ack
    //i2c_master_read(cmd, variable, 1-1, ACK_VAL); //Lee el puerto i2c para guardarlo en la varible del puntero
    i2c_master_read_byte(cmd, variable, NACK_VAL); //Lee el puerto i2c para guardarlo en la varible del puntero
    i2c_master_stop(cmd); //Detiene la comunicacion (Bit de stop)
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS); //Envia toda la estructura con todo lo anterior por los pines establecido de i2c del micro
    i2c_cmd_link_delete(cmd); //Elimina la estructura
}


/*========================================================================
Funcion: i2c_master_init
Descripcion: Inicializa el puerto I2C como maestro con resistencias de pullup en los pines que se definieron
Retorna el resultado de la operacion
========================================================================*/
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;       //Puerto I2C a utilizar - 0 o 1
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,                //Modo maestro
        .sda_io_num = I2C_MASTER_SDA_IO,        //Pin SDA
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    //Habilita pull up en SDA
        .scl_io_num = I2C_MASTER_SCL_IO,        //Pin SCL
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    //Habilita pull up en SCL
        .master.clk_speed = I2C_MASTER_FREQ_HZ, //Frecuencia de clock
        //.clk_flags = 0,  /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    //Establece la configuración con los datos previamente cargados
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    //si no hay error, instala el driver sin buffers porque es maestro y devuelve el valor de error correspondiente
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}