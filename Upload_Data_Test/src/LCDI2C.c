/*==================[ Inclusiones ]============================================*/
#include "../include/LCDI2C.h"
#include "../include/I2C.h"

/*==================[Prototipos de funciones]======================*/
void LCDI2C_init(void);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print( char * str );

//=========================== Definiciones ================================
#define ESP_SLAVE_ADDR_LCD 0x27                /*!< ESP32 slave address, you can set any 7bit value */

/*==================[Implementaciones]=================================*/

/*========================================================================
Funcion: lcdCommand
Descripcion: Envia al LCD I2C un comando de configuracion
Parametro de entrada:
       unsigned char cmnd: Valor en binario del comando a configurar
No retorna nada
========================================================================*/
void lcdCommand(unsigned char cmnd){
    //El modulo (pcf8574) que convierte el lcd en I2C no es mas que un multiplexor
    //Este modulo recibe 8 bits por i2c los cuales se visualizan en los 8 pines de conexion que posee
    //Estos pines estan conectados fisicamente con el display 16x2 de la siguiente manera
    //P0=RS P1=RW  P2=EN   P3=luz  P4=D4  P5=D5  P6=D6  P7=D7 
    //De esta forma el LCD debe trabajar en formato 4 Bits, donde es necesario pasar los 8 bits de datos dividio en 2 nibbles de 4 bits.
    //Como se observa, los primeros 4 bits del dato a enviar al modulo por i2c corresponden a RS, RW, EN, LUZ
    //Los 4 bits superiores corresponden al dato enviado al display.
    //RW se deja siempre en 0, mientras que RS es 1 cuando el dato a enviar es para imprimier en la pantalla o es 0 cuando el dato enviado
    //es de configuracion.
    //EN varia de 1 a 0 para validar los datos enviados. P3 se mantiene siempre en 1 para dejar prendida la luz del display
    //Por lo que se explico hasta recien, se observa a continuacion que se realizan mascaras para mantener los 4 bits inferiores en los 
    //valores que corresponden.

    //Envio los primeros 4 bits superiores de dato cmnd
    Send_i2c(((cmnd & 0xF0) & 0b11111100) | 0b00001100, ESP_SLAVE_ADDR_LCD);//RS = 0 para colocar dato 
                                                                            //RW = 0 para colocar en modo escritura
                                                                            //EN = 1 pulso en alto y en bajo para validar
                                                                            //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((cmnd & 0xF0) & 0b11111000 ) | 0b00001000, ESP_SLAVE_ADDR_LCD);           //EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide

    //Envio los 4 bits inferiores del dato cmnd
    Send_i2c(((cmnd <<4) & 0b11111100) | 0b00001100, ESP_SLAVE_ADDR_LCD);//RS = 0 para colocar dato 
                                                                        //RW = 0 para colocar en modo escritura
                                                                        //EN = 1 pulso en alto y en bajo para validar
                                                                        //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((cmnd << 4) & 0b11111000 ) | 0b00001000, ESP_SLAVE_ADDR_LCD);           //EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide
}

/*========================================================================
Funcion: lcdData
Descripcion: Envia al LCD I2C un dato para imprimir un caracter
Parametro de entrada:
       unsigned char cmnd: Valor en binario del caracter a imprimir
No retorna nada
========================================================================*/
void lcdData(unsigned char data)
{
    //Funciona de la misma forma que LcdComand pero en este caso, RS=1 siempre ya que lo que se envia es un dato para imprimir
    //Envio los primeros 4 bits superiores de dato data
    Send_i2c(((data & 0xF0) & 0b11111101) | 0b00001101, ESP_SLAVE_ADDR_LCD);//RS = 1 para colocar dato 
                                                                            //RW = 0 para colocar en modo escritura
                                                                            //EN = 1 pulso en alto y en bajo para validar
                                                                            //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((data & 0xF0) & 0b11111001) | 0b00001001, ESP_SLAVE_ADDR_LCD);//EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide


    //Envio los 4 bits inferiores del dato data
    Send_i2c(((data << 4) & 0b11111101) | 0b00001101, ESP_SLAVE_ADDR_LCD);//RS = 1 para colocar dato 
                                                                          //RW = 0 para colocar en modo escritura
                                                                          //EN = 1 pulso en alto y en bajo para validar
                                                                          //P3=1 luz encendida
    vTaskDelay(1 / portTICK_PERIOD_MS);                //espero para que se valide                               
    Send_i2c(((data << 4) & 0b11111001) | 0b00001001, ESP_SLAVE_ADDR_LCD);//EN = 0 pulso en alto y en bajo para validar dato
    vTaskDelay(1 / portTICK_PERIOD_MS);              //espero para que se valide
}


/*========================================================================
Funcion: LCDI2C_init
Descripcion: Inicializa y configura el LCD 
No retorna nada
========================================================================*/
void LCDI2C_init(){
    
    Send_i2c(0xFF, ESP_SLAVE_ADDR_LCD); //Limpio la salida de P0 a P7 produciendo el apagado de enable
	vTaskDelay(30 / portTICK_PERIOD_MS);  // Espera para iniciar
    lcdCommand(0x02); //Posiciona el puntero en 1,1 (HOME)
    lcdCommand(0x28); //Inicia. LCD 2 lineas, 5x7 matriz, modo 4 bits
	lcdCommand(0x0C); //Enciende display
    //lcdCommand(0x0E); //Cursor on
    lcdCommand(0x01); //Limpia LCD
	vTaskDelay(2 / portTICK_PERIOD_MS); //Espera
	lcdCommand(0x06); //Mueve el cursor a la derech
}

/*========================================================================
Funcion: lcd_gotoxy
Descripcion: Posiciona el cursor en la posicion indicada
Parametro de entrada:
        unsigned char x: Posicion en x donde se desea colocar el cursor
        unsigned char y: Posicion en y donde se desea colocar el cursor
No retorna nada
========================================================================*/
void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstCharAdr[] ={ 0x80,0xC0} ;//Comandos para poscicionarse Tabla 12-5
	lcdCommand (firstCharAdr[y-1] + x - 1); //Envia al display la configuracion del parametro obtenido
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

/*========================================================================
Funcion: lcd_print
Descripcion: Funcion para imprimir cadena de caracteres en el lcd
Parametro de entrada:
        char * str: recibe el puntero del string a imprimir
No retorna nada
========================================================================*/
void lcd_print( char * str )
{
	unsigned char i = 0;
	while (str[i] != 0) //Recorre todo el arreglo
	{
		lcdData(str[i]); //Imprime caracter por caracter
		i++ ;
	}
}
