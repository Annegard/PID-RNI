/******************************************************************************
   Adaptado del ejemplo de Platformio-ESP-IDF: http_request_example_main y de WiFi STA
******************************************************************************/

//==================== Inclusiones ==========================
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "openssl/ssl.h"

//==================== Definiciones ==========================
#define WIFI_SSID      "Nombre de la red"
#define WIFI_PASS      "Contraseña"
#define MAXIMUM_RETRY  2
// El event group permite múltiples bits para cada evento, pero solo nos interesan dos:
#define WIFI_CONNECTED_BIT BIT0 //está conectado al AP con una IP
#define WIFI_FAIL_BIT      BIT1 //no pudo conectarse después de la cantidad máxima de reintentos
#define THINGSPEAK_API_WRITE_KEY "XUK6QQH3ILI7JGIM"
#define WEB_SERVER "api.thingspeak.com"
#define WEB_PORT "80"

//================== Variables y constantes =======================
static EventGroupHandle_t s_wifi_event_group; //FreeRTOS event group para señalar cuando está conectado
static int s_retry_num = 0;
const static char *TAG = "Ejemplo_thingspeak_client";
static const char *REQUEST = "GET /update?api_key=XUK6QQH3ILI7JGIM&field1=%i\r\n"; //Solicitud para escribir el campo 1
int valor = 0;

//================== Prototipos =======================
static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void iniciarWifi(void);
static void tareaHttp(void *pvParameters);

//================== Función principal =======================
void app_main(void)
{
    iniciarWifi();
    
    esp_err_t ret = xTaskCreate(&tareaHttp, "tareaHttp", 4096, NULL, 5, NULL);
    if (ret != pdPASS)  {
        ESP_LOGI(TAG, "Error al crear tarea");
    }
}

//================== Funciones =======================
void iniciarWifi(void)
{
    esp_err_t ret = nvs_flash_init(); //Inicializa el almacenamiento no volatil (NVS)
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init()); //Inicializa el TCP/IP stack
    ESP_ERROR_CHECK(esp_event_loop_create_default()); //Crea un event loop

    //Crea WIFI STA por defecto. En caso de cualquier error de inicialización, 
    //esta API se cancela.
    //La API crea el objeto esp_netif con la configuración predeterminada de la estación WiFi,
    //adjunta el netif a wifi y registra los controladores de wifi predeterminados.
    esp_netif_create_default_wifi_sta(); 

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Parámetros de configuración del WiFi stack
    
    //Inicia el recurso WiFi Alloc para el controlador WiFi,
    // como la estructura de control WiFi, el búfer RX / TX, la estructura WiFi NVS, etc.
    // También inicia la tarea WiFi.
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id, instance_got_ip;

    //Registra una instancia del controlador de eventos en el bucle predeterminado.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &eventHandler,
                                                        NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &eventHandler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,          //Nombre de la red
            .password = WIFI_PASS,      //Contraseña
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK, //tipo de autenticación
            .pmf_cfg = {
                .capable = true, //Anuncia la compatibilidad con Protected Management Frame (PMF). El dispositivo preferirá conectarse en modo PMF si otro dispositivo también anuncia la capacidad PMF.
                .required = false //Anuncia que se requiere Protected Management Frame (PMF). El dispositivo no se asociará a dispositivos que no sean compatibles con PMF.
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) ); //Configura el modo STA, AP o STA+AP
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) ); //Configura el WiFI segun el modo seleccionado
    ESP_ERROR_CHECK(esp_wifi_start() ); //Inicia el Wifi
    ESP_LOGI(TAG, "Se finalizó la inicialización del WiFi.");
    /*
    Espera hasta que se establezca la conexión (WIFI_CONNECTED_BIT)
    o la conexión falle para el número máximo de reintentos (WIFI_FAIL_BIT).
    Los bits son establecidos por eventHandler ()
    */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() devuelve los bits antes de que se devolviera la llamada,
    por lo que podemos probar qué evento sucedió realmente. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado al AP SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Fallo al conectar al AP SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "Evento inesperado");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

static void eventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect(); //conecta el ESP al AP elegido
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

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


/*
//Para leer una respuesta del socket
int s, r;
char recv_buf[64];
//...
struct timeval receiving_timeout;
receiving_timeout.tv_sec = 5;
receiving_timeout.tv_usec = 0;
//SO_RCVTIMEO permite establecer un tiempo límite para la respuesta
if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) < 0) {
    ESP_LOGE(TAG, "... failed to set socket receiving timeout");
    close(s);
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    continue;
}
ESP_LOGI(TAG, "... set socket receiving timeout success");

//Lee la respuesta del servidor
do {
    bzero(recv_buf, sizeof(recv_buf));
    r = read(s, recv_buf, sizeof(recv_buf)-1);
    for(int i = 0; i < r; i++) {
        putchar(recv_buf[i]);
    }
} while(r > 0);

ESP_LOGI(TAG, "Se leyó correctamente desde el socket. Último dato leído = %d errno=%d.", r, errno);*/