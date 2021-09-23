#ifndef WIFI_Control_H_
#define WIFI_Control_H_
    
    #include "esp_log.h"
    #include "nvs_flash.h"
    #include "esp_netif.h"
    #include "esp_http_client.h"
    #include "esp_wifi.h"
    #include "lwip/sockets.h"
    #include "lwip/netdb.h"
    #include "openssl/ssl.h"
    #include "freertos/event_groups.h"
    
    //==================== Definiciones ==========================
    #define WIFI_SSID      "red"
    #define WIFI_PASS      "contraseña"
    #define MAXIMUM_RETRY  2
    // El event group permite múltiples bits para cada evento, pero solo nos interesan dos:
    #define WIFI_CONNECTED_BIT BIT0 //está conectado al AP con una IP
    #define WIFI_FAIL_BIT      BIT1 //no pudo conectarse después de la cantidad máxima de reintentos
    #define THINGSPEAK_API_WRITE_KEY "KPC7Y114VMMZOF59"
    #define WEB_SERVER "api.thingspeak.com"
    #define WEB_PORT "80"
    
    //================== Prototipos =======================
    void iniciarWifi();

#endif