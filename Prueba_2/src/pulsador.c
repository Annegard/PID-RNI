/*==================[ Inclusiones ]============================================*/
#include "pulsador.h"

/*==================[ Definiciones ]===================================*/

#define T_REBOTE_MS     40
#define T_REBOTE        pdMS_TO_TICKS(T_REBOTE_MS)

/*==================[Prototipos de funciones]======================*/

static void errorPulsador( uint8_t  indice );
static void botonLiberado( uint8_t  indice );

void tareaPulsadores( void* taskParmPtr );

/*==================[Variables]==============================*/
gpio_int_type_t pulsadorPines[N_PULSADOR] = {GPIO_NUM_12, GPIO_NUM_14};

pulsadorInfo pulsador [N_PULSADOR];

/*==================[Implementaciones]=================================*/
TickType_t obtenerDiferencia(uint8_t  indice)
{
    TickType_t tiempo;
    tiempo = pulsador[indice].diferenciaTiempo;
    return tiempo;
}

void borrarDiferencia( uint8_t  indice )
{
    pulsador[indice].diferenciaTiempo = TIEMPO_NO_VALIDO;
}

void inicializarPulsador( void )
{
    for(int i = 0; i < N_PULSADOR; i++)
    {
        pulsador[i].tecla             = pulsadorPines[i];
        pulsador[i].estado            = ALTO;                     //Estado inicial
        pulsador[i].tiempoBajo        = TIEMPO_NO_VALIDO;
        pulsador[i].tiempoAlto        = TIEMPO_NO_VALIDO;
        pulsador[i].diferenciaTiempo  = TIEMPO_NO_VALIDO;
        pulsador[i].semaforo          = xSemaphoreCreateBinary();

        gpio_pad_select_gpio(pulsador[i].tecla);
        gpio_set_direction(pulsador[i].tecla , GPIO_MODE_INPUT);
        gpio_set_pull_mode(pulsador[i].tecla, GPIO_PULLDOWN_ONLY); //Habilita resistencia de PULLDOWN interna
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // Crear tareas en freeRTOS
    BaseType_t res = xTaskCreatePinnedToCore(
    	tareaPulsadores,                     	// Funcion de la tarea a ejecutar
        "tareaPulsadores",   	                // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*2, 		// Cantidad de stack de la tarea
        NULL,                          	// Parametros de tarea
        tskIDLE_PRIORITY+2,         	// Prioridad de la tarea -> Queremos que este un nivel encima de IDLE
        NULL,                          		// Puntero a la tarea creada en el sistema
        PROCESADORA);

    // Gestion de errores
	if(res == pdFAIL)
	{
		printf( "Error al crear la tarea.\r\n" );
		while(true);					// si no pudo crear la tarea queda en un bucle infinito
	}
}

static void errorPulsador( uint8_t  indice )
{
    pulsador[indice].estado = ALTO;
}

// pulsador_ Update State Function
void actualizarPulsador( uint8_t  indice)
{
    switch( pulsador[indice].estado )
    {
        case BAJO:
            if( gpio_get_level( pulsador[indice].tecla ) ){
                pulsador[indice].estado = ASCENDENTE;
            }
            break;

        case ASCENDENTE:
            if( gpio_get_level( pulsador[indice].tecla ) ){
                pulsador[indice].estado = ALTO;
            }
            else{
                pulsador[indice].estado = BAJO;
            }
            break;

        case ALTO:
            if( !gpio_get_level( pulsador[indice].tecla ) ){
                pulsador[indice].estado = DESCENDENTE;
            }
            break;

        case DESCENDENTE:
            if( !gpio_get_level( pulsador[indice].tecla ) ){
                pulsador[indice].estado = BAJO;
                botonLiberado(indice);
            }
            else{
                pulsador[indice].estado = ALTO;
            }
            break;

        default:
            errorPulsador(indice);
            break;
    }
}

/* accion de el evento de tecla liberada */
static void botonLiberado( uint8_t  indice)
{
    xSemaphoreGive (pulsador[indice].semaforo); //libera el semaforo
}

void tareaPulsadores( void* taskParmPtr )
{
    while( true )
    {
        for (int i = 0; i < N_PULSADOR; i++)
        {
            actualizarPulsador(i);
        }
        vTaskDelay( T_REBOTE );
    }
}