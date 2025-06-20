#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "hardware/adc.h"
#include "lwip/tcp.h"

#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/display.h"
#include "lib/led/led.h"
#include "lib/button/button.h"
#include "lib/matrix_leds/matrix_leds.h"
#include "lib/buzzer/buzzer.h"
#include "lib/ultrasonic/ultrasonic.h"
#include "config/wifi_config.h"
#include "public/html_data.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define RELE_PIN 16 // Gpio que ativará(low) e desativará(high) o relé para acionar a bomba
#define ADC_PIN_POTENTIOMETER_READ 28 // Pino do ADC para ler os valores alterados no potencimetro pela boia
#define ULTRASONIC_TRIG_PIN 18 // Pino do Trig do sensor ultrassônico
#define ULTRASONIC_ECHO_PIN 19 // Pino do Echo do sensor ultrassônico

// Definições para o sensor ultrassônico
#define ULTRASONIC_DIST_MAX_VAZIO 28.0f // Distância máxima lida (reservatório vazio)
#define ULTRASONIC_DIST_MIN_CHEIO 15.0f // Distância mínima lida (reservatório cheio)


// OBS: Na hora da montagem do reservatório precisamos ver até onde o potênciometro irá girar no nível mínimo  e máximo para ajustar os valores abaixo
// Definições para o potenciômetro de boia
#define ADC_MIN_POTENTIOMETER_READING 22   // Valor mínimo lido do potenciômetro (quando o reservatório está vazio)
#define ADC_MAX_POTENTIOMETER_READING 4070 // Valor máximo lido do potenciômetro (quando o reservatório está cheio)

// Fila para armazenar os valores de nivel de agua lidos
QueueHandle_t xQueueWaterLevelReadings;
SemaphoreHandle_t xMutexDisplay;
// Estrutura de dados
struct http_state
{
    char response[10000];
    size_t len;
    size_t sent;
};

// Prototipos das funções
void vWebServerTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vControlWaterPumpTask(void * pvParameters);
void vReadPotentiometerTask(void *pvParameters);
void vUltrasonicSensorTask(void *pvParameters);
void vMatrixLedsTask(void *pvParameters);
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void start_http_server(void);
void button_callback(uint gpio, uint32_t events);

// Variáveis globais
ssd1306_t ssd; // Declaração do display OLED
volatile static int min_water_level_limit = 10; // Limite mínimo do nível de água (em porcentagem)
volatile static int max_water_level_limit = 50; // Limite máximo do nível de água (em porcentagem)
volatile static bool reset_limits=false;
float ultrasonic_distance = 0.0f; // Variável para armazenar a distância medida pelo sensor ultrassônico
int main()
{
    stdio_init_all();
    sleep_ms(2000); // Aguarda a serial se conectar

    button_init_predefined(true, true, true);

    button_setup_irq(true,true,true,button_callback);

    // Inicializa o display OLED
    init_display(&ssd);

    init_leds();

    ssd1306_fill(&ssd, false); // Limpa a tela
    ssd1306_send_data(&ssd);   // Envia os dados para o display

    xQueueWaterLevelReadings = xQueueCreate(5, sizeof(int));
    xMutexDisplay = xSemaphoreCreateMutex();

    xTaskCreate(vWebServerTask, "WebServerTask", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vControlWaterPumpTask, "AcionaBombaComBaseNoNivelTask", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "vMostraDadosNoDisplayTask", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vReadPotentiometerTask, "LeituraPotenciometroTask", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);
    //xTaskCreate(vUltrasonicSensorTask, "vUltrasonicSensorTask", configMINIMAL_STACK_SIZE,
    //            NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrixLedsTask, "vMatrixLedsTask", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

void button_callback(uint gpio, uint32_t events){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if(current_time - last_debounce_time > debounce_delay){

        if(gpio == BUTTON_A){
            reset_limits=true;
        }
        else if(gpio == BUTTON_B){

        }
        else if(gpio == BUTTON_SW){

        }

        last_debounce_time = current_time;
    }
}

// Task que controla o sensor ultrassônico
void vUltrasonicSensorTask(void *pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado

    // Inicializa os pinos do sensor ultrassônico
    setup_ultrasonic_pins(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
    while (true){
        uint64_t pulse_duration = get_pulse_duration_us(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);

        if (pulse_duration > 0) {
            float distance_cm = microseconds_to_cm(pulse_duration);
            float distance_inches = microseconds_to_inches(pulse_duration);

            printf("Distância: %.2f cm (%.2f inches)\n", distance_cm, distance_inches);
            ultrasonic_distance = distance_cm; // Armazena a distância medida
        } else {
            printf("Erro: Timeout. Nenhum objeto detectado no alcance.\n");
        }

         // Converte a distância para porcentagem do nível de água
        // A lógica é inversa: quanto MAIOR a distância, mais VAZIO o tanque (0%).
        // Quanto MENOR a distância, mais CHEIO o tanque (100%).
        float float_ultrassonic_distance = (float) ultrasonic_distance;
        float range = ULTRASONIC_DIST_MAX_VAZIO - ULTRASONIC_DIST_MIN_CHEIO;
        float adjusted_distance = ultrasonic_distance - ULTRASONIC_DIST_MIN_CHEIO; // Ajusta a distância para o novo zero (tanque cheio)

        // Calcula a porcentagem.
        // Se a distância for ULTRASONIC_DIST_MIN_CHEIO (cheio), adjusted_distance será 0, e a porcentagem será 100.
        // Se a distância for ULTRASONIC_DIST_MAX_VAZIO (vazio), adjusted_distance será 'range', e a porcentagem será 0.
        int water_level_percentage = (int)(((range - adjusted_distance) / range) * 100.0f);

        // Garante que a porcentagem esteja entre 0 e 100
        if (water_level_percentage < 0.) {
            water_level_percentage = 0;
        } else if (water_level_percentage > 100) {
            water_level_percentage = 100;
        }

        xQueueSend(xQueueWaterLevelReadings, &water_level_percentage, 0); // Envia o valor da porcentagem para a fila
        vTaskDelay(pdMS_TO_TICKS(250)); // Aguarda 500ms para a próxima leitura (ajustável)
    }
}

// Task que controla a leitura do joystick
void vReadPotentiometerTask(void *pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado
    // Inicializa o adc e os pinos responsáveis pelo eixo x e y do joystick
    adc_gpio_init(ADC_PIN_POTENTIOMETER_READ);
    adc_init();
    int water_level_percentage = 0;
    while (true){
        // ja estou armazenando na fila a porcentagem em relação ao valor lido
        // 0% == 22 lido pelo adc
        // 100% == 4077
        adc_select_input(2); // GPIO 26 = ADC0
        water_level_percentage = (((float)(adc_read() - ADC_MIN_POTENTIOMETER_READING) / (ADC_MAX_POTENTIOMETER_READING - ADC_MIN_POTENTIOMETER_READING)) * 100.0f);
        xQueueSend(xQueueWaterLevelReadings, &water_level_percentage, 0); // Envia o valor da leitura do potenciometro em porcentagem para fila
        vTaskDelay(pdMS_TO_TICKS(100));              // 10 Hz de leitura
    }
}


// Task que controla o relé da bomba de água
void vControlWaterPumpTask(void * pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado
    gpio_init(RELE_PIN);
    gpio_set_dir(RELE_PIN,GPIO_OUT);
    gpio_put(RELE_PIN,1);// Começa com o Relé desligado, pois ele no nivel alto da gpio é desligado ja que é um rele com optoacoplador
    int water_level_percentage = 0;
    while (true){
        if (xQueueReceive(xQueueWaterLevelReadings, &water_level_percentage, portMAX_DELAY) == pdTRUE){
            if (water_level_percentage <= min_water_level_limit){
                gpio_put(RELE_PIN,0); // Ativa o rele deixando os 12v passarem para a bomba
            }
            else if (water_level_percentage >= max_water_level_limit){
                gpio_put(RELE_PIN,1); // Desativa o rele desligando a bomba

            }

        }

        if (!gpio_get(RELE_PIN) && gpio_get(RED_LED_PIN)){
            set_led_green();

            play_tone(BUZZER_A_PIN, 300);
            vTaskDelay(pdMS_TO_TICKS(250)); // Toca o buzzer por 250ms
            stop_tone(BUZZER_A_PIN);

        }else if(gpio_get(RELE_PIN) && !gpio_get(RED_LED_PIN)){
            set_led_yellow();
            for (uint8_t i = 0; i < 2; i++)
            {
                play_tone(BUZZER_A_PIN, 900);
                vTaskDelay(pdMS_TO_TICKS(150)); // Toca o buzzer por 250ms
                stop_tone(BUZZER_A_PIN);
                vTaskDelay(pdMS_TO_TICKS(150)); // Toca o buzzer por 250ms
            }

        }

    }
}

// Task que exibe os dados no display OLED
void vDisplayTask(void *pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado
    int water_level_percentage = 0;
    char water_level_str[5]; // Buffer para armazenar a string
    char distance_str[10]; // Buffer para armazenar a string da distância
    while (true){
        if (xQueueReceive(xQueueWaterLevelReadings, &water_level_percentage, portMAX_DELAY) == pdTRUE){
            if (xSemaphoreTake(xMutexDisplay,portMAX_DELAY) == pdTRUE){
                sprintf(water_level_str, "%d%%", water_level_percentage); // Formata com '%'
                ssd1306_fill(&ssd, false);                     // Limpa o display
                ssd1306_rect(&ssd, 3, 3, 122, 60, true, false); // Desenha um retângulo
                ssd1306_line(&ssd, 3, 25, 123, 25, true);      // Desenha uma linha

                ssd1306_draw_string(&ssd, "MEDIDOR NIVEL", 8, 6); // Desenha uma string
                ssd1306_draw_string(&ssd, " DE AGUA", 20, 16);  // Desenha uma string
                ssd1306_draw_string(&ssd, "Nivel: ", 10, 31);           // Desenha uma string
                ssd1306_draw_string(&ssd, water_level_str, 58, 31);         // Desenha uma string
                ssd1306_draw_string(&ssd, gpio_get(RELE_PIN) ? "Bomba: OFF" : "Bomba: ON", 10,41);
                sprintf(distance_str, "%.2f cm", ultrasonic_distance); // Formata a distância medida
                ssd1306_draw_string(&ssd, distance_str, 20, 53); // Desenha a distância medida*/
                ssd1306_send_data(&ssd);                             // Atualiza o display
                xSemaphoreGive(xMutexDisplay);
            }

        }
    }
}

// Task que controla a matriz de LEDs
void vMatrixLedsTask(void *pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado
    init_led_matrix();
    apaga_matriz();
    int water_level_percentage = 0;
    while (true)
    {
        if(xQueueReceive(xQueueWaterLevelReadings, &water_level_percentage, portMAX_DELAY) == pdTRUE){
            if (water_level_percentage >= 80){
                desenha_frame(levels,4);
            }else if(water_level_percentage >= 60){
                desenha_frame(levels,3);
            }else if(water_level_percentage >= 40){
                desenha_frame(levels,2);
            }else if(water_level_percentage >= 20){
                desenha_frame(levels,1);
            }else{
                desenha_frame(levels,0);
            }

        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }


}

// Task que inicia o servidor web
void vWebServerTask(void *pvParameters){
    (void)pvParameters; // Evita aviso de parâmetro não utilizado

    if (xSemaphoreTake(xMutexDisplay,portMAX_DELAY) == pdTRUE){
        // Inicializa a biblioteca CYW43 para Wi-Fi
        if (cyw43_arch_init())
        {
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "WiFi => FALHA", 0, 0);
            ssd1306_send_data(&ssd);

            vTaskDelete(NULL);  // Encerrar esta task
        }

        cyw43_arch_enable_sta_mode();
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
        {
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "WiFi => ERRO", 0, 0);
            ssd1306_send_data(&ssd);

            vTaskDelete(NULL);  // Encerrar esta task
        }

        uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        char ip_str[24];
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        printf("IP: %s\n",ip_str);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "WiFi => OK", 0, 0);
        ssd1306_draw_string(&ssd, ip_str, 0, 10);
        ssd1306_send_data(&ssd);
        start_http_server();
        vTaskDelay(pdMS_TO_TICKS(2000)); // pra dar tempo de ver o ip
        xSemaphoreGive(xMutexDisplay);
    }


    while (1){
        // Mantém o servidor HTTP ativo
        cyw43_arch_poll();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
    }
     cyw43_arch_deinit();// Esperamos que nunca chegue aqui
}

// Função de callback para enviar dados HTTP
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

// Função de recebimento HTTP
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{   

    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs)
    {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    if (strstr(req, "GET /bomba/on")){
        gpio_put(RELE_PIN, 0); // Ativa o relé (LOW liga a bomba)
        const char *txt = "Bomba Ligada";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "%s",
                        (int)strlen(txt), txt);
    }
    else if (strstr(req, "GET /bomba/off")){
        gpio_put(RELE_PIN, 1); // Desativa o relé (HIGH desliga a bomba)
        const char *txt = "Bomba Desligada";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "%s",
                        (int)strlen(txt), txt);
    }
    else if (strstr(req, "GET /estado")){  // Se a requisição for para obter o estado dos sensores(potenciometro com boia)

        // ULTRASSONICO
        // Converte a distância ultrassônica para porcentagem
        /*
        float range = ULTRASONIC_DIST_MAX_VAZIO - ULTRASONIC_DIST_MIN_CHEIO;
        float adjusted_distance = ultrasonic_distance - ULTRASONIC_DIST_MIN_CHEIO;
        float nivel_porcentagem = (((range - adjusted_distance) / range) * 100.0f);

        // Garante que a porcentagem esteja entre 0 e 100 para o HTTP
        if (nivel_porcentagem < 0.0f) {
            nivel_porcentagem = 0.0f;
        } else if (nivel_porcentagem > 100.0f) {
            nivel_porcentagem = 100.0f;
        }
        */

        // //POTENCIOMETRO
        float nivel_porcentagem = (((float)(adc_read() - ADC_MIN_POTENTIOMETER_READING) / (ADC_MAX_POTENTIOMETER_READING - ADC_MIN_POTENTIOMETER_READING)) * 100.0f);
        // //

        int estado_bomba_para_json = !gpio_get(RELE_PIN);
        char json_payload[96]; // Buffer para a string JSON
        int json_len = snprintf(json_payload, sizeof(json_payload),
                                 "{\"bomba_agua\":%d,\"nivel_agua\":%d, \"limite_maximo\":%d,\"limite_minimo\":%d}\r\n",
                                 estado_bomba_para_json, (int)nivel_porcentagem, 
                                 max_water_level_limit, min_water_level_limit); // Enviando como 'nivel_agua', estado'bomba_agua', limite 'max' e 'min'
         hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           json_len, json_payload);

    }
    else if (strstr(req, "POST /limites")) {
        char *body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;
            
            // Extrai os valores diretamente usando sscanf
            int max_val, min_val;
            if(sscanf(body, "{\"max\":%d,\"min\":%d", &max_val, &min_val) == 2) {
                // Valida os valores recebidos
                if(max_val >= 0 && max_val <= 100 && min_val >= 0 && min_val <= 100) {
                    max_water_level_limit = max_val;
                    min_water_level_limit = min_val;
                }
            }
        }
        
        printf("Novos limites: Max=%d, Min=%d\n", 
            max_water_level_limit, 
            min_water_level_limit); 
        
        // Confirma atualização
        const char *txt = "Limites atualizados";
        hs->len = snprintf(hs->response, sizeof(hs->response),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n"
                        "%s",
                        (int)strlen(txt), txt);
    }
    else{
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(html_data), html_data);
    }

    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);

    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}

// Função de callback para aceitar conexões TCP
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Função para iniciar o servidor HTTP
static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

