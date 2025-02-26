#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"
#include "ssd1306.h"
#include "font.h"
#include "pico/time.h"  
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "projeto_final.pio.h"
#include "ws2812.pio.h"

// Definições de pinos do led
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11

// Botão B para colocar a placa em modo de boot
#define BUTTON_B_PIN 6

// Número de LEDs na matriz 5x5
#define NUM_PIXELS 25

// Pino de saída para a matriz de LEDs
#define OUT_PIN 7

// Configurações do I2C para o display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDRESS 0x3C

// Pino do joystick (eixo Y) para simular o sensor ultrassônico
#define JOYSTICK_Y_PIN 27

// Tempo de debounce para o botão
#define DEBOUNCE_DELAY_MS 200

// Dimensões do display OLED
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// Variáveis globais
ssd1306_t display;  
volatile uint32_t last_interrupt_time = 0;  // Último tempo de interrupção para debounce

// Função de interrupção para o botão B colocar a placa em modo de boot
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if ((current_time - last_interrupt_time) > DEBOUNCE_DELAY_MS) {
        last_interrupt_time = current_time;
        if (gpio == BUTTON_B_PIN) {
            reset_usb_boot(0, 0);  // Coloca a placa em modo de boot
        }
    }
}

// Rotina para definição da intensidade de cores do LED
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;  // Converte o valor de vermelho para 8 bits
    G = g * 255;  // Converte o valor de verde para 8 bits
    B = b * 255;  // Converte o valor de azul para 8 bits
    return (G << 24) | (R << 16) | (B << 8);  // Combina os valores em um único inteiro
}

// Rotina para acionar a matriz de LEDs WS2812B
void desenho_pio(double desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (i < (desenho * 25)) {    
            valor_led = matrix_rgb(b=0.1, r=0.1, g=0.1);  // Define a cor dos LEDs acesos
            pio_sm_put_blocking(pio, sm, valor_led);  // Envia o valor para o PIO
        } else {
            valor_led = matrix_rgb(b=0.0, r=0.0, g=0.0);  // Define a cor dos LEDs apagados
            pio_sm_put_blocking(pio, sm, valor_led);  // Envia o valor para o PIO
        }
    }
}

// Função para gerar um timestamp no formato ISO 8601
void gerar_timestamp(char *timestamp, size_t tamanho) {
    // Obtém o tempo atual em segundos desde o início da execução
    uint64_t tempo_atual = to_us_since_boot(get_absolute_time()) / 1000000;

    // Simula uma data fixa (ex.: 2025-02-25) e adiciona o tempo de execução
    uint64_t segundos_por_dia = 86400; // Segundos em um dia
    uint64_t dias_decorridos = tempo_atual / segundos_por_dia;
    uint64_t segundos_restantes = tempo_atual % segundos_por_dia;

    // Data de referência: 2023-10-25T00:00:00Z
    snprintf(timestamp, tamanho, "2025-02-%02lldT%02lld:%02lld:%02lldZ",
             25 + dias_decorridos, // Dia
             segundos_restantes / 3600, // Horas
             (segundos_restantes % 3600) / 60, // Minutos
             segundos_restantes % 60); // Segundos
}

// Função para enviar dados simulados para a central
void enviar_dados_central(uint16_t nivel_lixeira) {
    char timestamp[25];
    gerar_timestamp(timestamp, sizeof(timestamp));
    printf("{\"id_lixeira\": \"001\", \"nivel\": %d%%, \"timestamp\": \"%s\"}\n", nivel_lixeira, timestamp);
}


int main() {
    stdio_init_all();  // Inicializa a comunicação serial para debug

    PIO pio = pio0;  // Usa o PIO0 para controlar a matriz de LEDs
    uint16_t i;
    uint32_t valor_led;
    double r = 0.0, b = 0.0, g = 0.0;

    // Configurações da PIO para controlar a matriz de LEDs
    uint offset = pio_add_program(pio, &projeto_final_program);
    uint sm = pio_claim_unused_sm(pio, true);
    projeto_final_program_init(pio, sm, offset, OUT_PIN);
    
    // Inicialização dos GPIOs
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configuração das interrupções para o botão B
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicialização do display OLED
    i2c_init(I2C_PORT, 400 * 1000);  // Inicializa o I2C com clock de 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&display, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_ADDRESS, I2C_PORT);
    ssd1306_config(&display);  // Configura o display
    ssd1306_send_data(&display);
    ssd1306_fill(&display, false);  // Limpa o display
    ssd1306_send_data(&display);

    // Inicialização do ADC para ler o joystick
    adc_init();
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Loop principal
    while (true) {
        // Lê o valor do eixo Y do joystick
        adc_select_input(0);
        uint16_t y_value = adc_read();  // Lê o valor do ADC
        uint16_t nivel_lixeira = y_value / 40.95;  // Converte o valor para porcentagem (0-100%)
        double desenho_nivel_lixeira = nivel_lixeira / 100.0;  // Normaliza o valor para a matriz de LEDs

        // Atualiza a matriz de LEDs com o nível da lixeira
        desenho_pio(desenho_nivel_lixeira, valor_led, pio, sm, r, g, b);

        // Controla os LEDs RGB e o display com base no nível da lixeira
        if (nivel_lixeira < 50) {
            gpio_put(LED_GREEN_PIN, true);  // LED verde aceso
            gpio_put(LED_RED_PIN, false);   // LED vermelho apagado
            ssd1306_fill(&display, false);
            ssd1306_draw_string(&display, "NAO COLETAR", 8, 10);  // Exibe mensagem no display        
        } else if (nivel_lixeira >= 50 && nivel_lixeira < 75) {
            gpio_put(LED_GREEN_PIN, true);  // LED verde aceso
            gpio_put(LED_RED_PIN, true);    // LED vermelho aceso (amarelo)
            ssd1306_fill(&display, false);
            ssd1306_draw_string(&display, "COLETAR TALVEZ", 8, 10);  // Exibe mensagem no display
        } else {
            gpio_put(LED_GREEN_PIN, false);  // LED verde apagado
            gpio_put(LED_RED_PIN, true);     // LED vermelho aceso
            ssd1306_fill(&display, false);
            ssd1306_draw_string(&display, "COLETAR", 8, 10);  // Exibe mensagem no display
        }
        // Envia os dados simulados para a central
        enviar_dados_central(nivel_lixeira);
        ssd1306_send_data(&display); // simula a decisão da central
        
        sleep_ms(500);  // Aguarda 500ms antes da próxima leitura
    }
    return 0;
}