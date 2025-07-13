#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_crt_bundle.h"
#include "sdkconfig.h"

#define BOTAO_GPIO GPIO_NUM_12
static const char *TAG_APP = "PUB_LED_MQTT";

// Configura o pino do botão como entrada com pull-up
static void configurar_botao_gpio(void)
{
    gpio_reset_pin(BOTAO_GPIO);
    gpio_set_direction(BOTAO_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOTAO_GPIO, GPIO_PULLUP_ONLY);
    ESP_LOGI(TAG_APP, "GPIO %d configurado como entrada para o botão.", BOTAO_GPIO);
}

// ====================================================================
// NOVA VERSÃO DA TASK - LÓGICA DE INTERRUPTOR
// ====================================================================
void task_monitorar_botao(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)pvParameters;
    const char *TOPICO_LED = "/ifpe/ads/embarcados/esp32/led";
    
    int estado_anterior = 1;
    static int led_ligado = 0; // Variável para guardar o estado do LED (0=desligado, 1=ligado)

    while (1)
    {
        int estado_atual = gpio_get_level(BOTAO_GPIO);

        // Detecta APENAS quando o botão é pressionado (borda de descida)
        if (estado_atual == 0 && estado_anterior == 1)
        {
            // Inverte o estado do LED
            led_ligado = !led_ligado; 

            if (led_ligado) 
            {
                ESP_LOGI(TAG_APP, "Toque! Publicando '1' (LIGA)...");
                esp_mqtt_client_publish(client, TOPICO_LED, "1", 0, 1, 0);
            } 
            else 
            {
                ESP_LOGI(TAG_APP, "Toque! Publicando '0' (DESLIGA)...");
                esp_mqtt_client_publish(client, TOPICO_LED, "0", 0, 1, 0);
            }
        }

        estado_anterior = estado_atual;
        // O delay agora serve para evitar múltiplos toques acidentais.
        vTaskDelay(pdMS_TO_TICKS(200)); 
    }
}

// Manipulador de eventos MQTT
static void manipulador_eventos_mqtt(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_APP, "MQTT conectado. Iniciando monitoramento do botão...");
        xTaskCreate(task_monitorar_botao, "MonitorarBotao", 4096, client, 5, NULL);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG_APP, "MQTT desconectado.");
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG_APP, "Erro no cliente MQTT.");
        break;

    default:
        break;
    }
}

static void iniciar_cliente_mqtt(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_BROKER_URL,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, manipulador_eventos_mqtt, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG_APP, "Cliente MQTT inicializado.");
}

void app_main(void)
{
    ESP_LOGI(TAG_APP, "Inicializando aplicativo MQTT Publicador de LED...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    configurar_botao_gpio();
    iniciar_cliente_mqtt();
}