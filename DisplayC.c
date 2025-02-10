#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define MAX_CARACTERES 40  // Define o tamanho máximo do buffer para caracteres

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW false

#define BOTAO_A 5
#define BOTAO_B 6
#define LED_VERDE 11
#define LED_AZUL 12

PIO pio = pio0;
uint sm = 0;
bool estado_led_verde = false;
bool estado_led_azul = false;
ssd1306_t ssd;
absolute_time_t last_interrupt_time_a = {0};
absolute_time_t last_interrupt_time_b = {0};
const uint32_t debounce_delay_ms = 200;
// Buffer para armazenar os caracteres recebidos
char buffer_recebido[MAX_CARACTERES + 1] = "";  // +1 para o caractere nulo de terminação

void display_welcome_message() {
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "Kaique Rangel", 10, 10);
    ssd1306_draw_string(&ssd, "Embarcatech", 10, 25);
    ssd1306_draw_string(&ssd, "Tarefa", 10, 40);
    ssd1306_send_data(&ssd);
}

// Função para configurar um pixel da matriz de LEDs
void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

// Inverte a matriz 5x5 em 180 graus
void invert_matrix_180(uint8_t *number) {
    uint8_t inverted[NUM_PIXELS];
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            inverted[(4 - row) * 5 + col] = number[row * 5 + col];
        }
    }
    for (int i = 0; i < NUM_PIXELS; i++) {
        number[i] = inverted[i];
    }
}

// Mapeamento dos números 0-9 para matriz 5x5
const uint8_t numbers[10][NUM_PIXELS] = {
   // Número 0
   {0, 1, 1, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0},
   // Número 1
   {0, 0, 1, 0, 0,
    0, 1, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0},
   // Número 2
   {0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 0, 0, 0,
    0, 1, 1, 1, 0},
   // Número 3
   {0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 1, 1, 0},
   // Número 4
   {0, 1, 0, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 0, 0, 0},
   // Número 5
   {0, 1, 1, 1, 0,
    0, 1, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 1, 1, 0},
   // Número 6
   {0, 1, 1, 1, 0,
    0, 1, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0},
   // Número 7
   {0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0},
   // Número 8
   {0, 1, 1, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0},
   // Número 9
   {0, 1, 1, 1, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 0, 0, 0}
};

// Exibir número na matriz de LEDs
void display_number(int number) {
    uint8_t temp[NUM_PIXELS];
    for (int i = 0; i < NUM_PIXELS; i++) {
        temp[i] = numbers[number][i];
    }
 
    invert_matrix_180(temp);
    float brightness_factor = 0.05;

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t green_color = 0xFF0000;
        uint8_t r = (green_color >> 16) & 0xFF;
        uint8_t g = (green_color >> 8) & 0xFF;
        uint8_t b = green_color & 0xFF;

        r = (uint8_t)(r * brightness_factor);
        g = (uint8_t)(g * brightness_factor);
        b = (uint8_t)(b * brightness_factor);

        uint32_t dimmed_color = (r << 16) | (g << 8) | b;
        set_pixel(i, temp[i] ? dimmed_color : 0x000000);
    }
}

// Função de debounce
bool debounce(absolute_time_t *last_interrupt_time) {
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(*last_interrupt_time, current_time) / 1000 < debounce_delay_ms) {
        return false;
    }
    *last_interrupt_time = current_time;
    return true;
}
// Tratamento da interrupção
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A && debounce(&last_interrupt_time_a)) {
        estado_led_verde = !estado_led_verde;
        gpio_put(LED_VERDE, estado_led_verde);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, estado_led_verde ? "LED Verde ON" : "LED Verde OFF", 10, 30);
        ssd1306_send_data(&ssd);
        printf("Botão A pressionado. LED Verde %s\n", estado_led_verde ? "ON" : "OFF");
    }

    if (gpio == BOTAO_B && debounce(&last_interrupt_time_b)) {
        estado_led_azul = !estado_led_azul;
        gpio_put(LED_AZUL, estado_led_azul);
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, estado_led_azul ? "LED Azul ON" : "LED Azul OFF", 10, 30);
        ssd1306_send_data(&ssd);
        printf("Botão B pressionado. LED Azul %s\n", estado_led_azul ? "ON" : "OFF");
    }
}

int main() {
    stdio_init_all();

    // Inicializa I2C e display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa a matriz WS2812
    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);

    // Configuração dos botões e LEDs
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, 0);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

     // Exibir mensagem inicial
    display_welcome_message();
    char recebido;
    bool cor = true;
    while (true) {
        int caracter_recebido = getchar_timeout_us(100000);
        if (caracter_recebido != PICO_ERROR_TIMEOUT) {
            recebido = (char)caracter_recebido;
            
            // Adiciona o caractere ao buffer se não tiver excedido o limite
            if (strlen(buffer_recebido) < MAX_CARACTERES) {
                int len = strlen(buffer_recebido);
                buffer_recebido[len] = recebido;
                buffer_recebido[len + 1] = '\0';  // Adiciona o terminador nulo
            }
    
            // Atualiza o display com a string acumulada
            ssd1306_fill(&ssd, false);  // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, buffer_recebido, 10, 30);  // Exibe a string
            ssd1306_send_data(&ssd);  // Envia os dados ao display
    
            // Se o caractere for um número, exibe o número na matriz de LEDs
            if (recebido >= '0' && recebido <= '9') {
                display_number(recebido - '0');
            }
        }
        sleep_ms(10);
    }
}