#include "WIFI_Control.h"

//================== Variables y constantes =======================
static EventGroupHandle_t s_wifi_event_group; //FreeRTOS event group para señalar cuando está conectado
static int s_retry_num = 0;
char *TAG_aux = "Ejemplo_thingspeak_client";

//================== Prototipos =======================
static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void iniciarWifi()
{
    esp_err_t ret = nvs_flash_init(); //Inicializa el almacenamiento no volatil (NVS)
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG_aux, "ESP_WIFI_MODE_STA");
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
    ESP_LOGI(TAG_aux, "Se finalizó la inicialización del WiFi.");
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
        ESP_LOGI(TAG_aux, "Conectado al AP SSID:%s password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_aux, "Fallo al conectar al AP SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PASS);
    } else {
        ESP_LOGE(TAG_aux, "Evento inesperado");
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
            ESP_LOGI(TAG_aux, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_aux,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_aux, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

