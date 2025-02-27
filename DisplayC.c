#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

// Definição de pinos e endereços I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C  // Endereço do display OLED SSD1306

// Definição para LEDs WS2812
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW false  // O LED não tem canal adicional W

// Definição de pinos para os botões e LEDs
#define BOTAO_A 5
#define BOTAO_B 6
#define LED_VERDE 11
#define LED_AZUL 12

// Variáveis globais para o PIO e a máquina de estado
PIO pio = pio0;
uint sm = 0;
ssd1306_t ssd;  // Estrutura para controlar o display OLED
absolute_time_t last_interrupt_time_a = {0};  // Controle de debounce para o botão A
absolute_time_t last_interrupt_time_b = {0};  // Controle de debounce para o botão B
const uint32_t debounce_delay_ms = 200;  // Tempo de debounce em milissegundos

// Definição de sequência e controle de progresso
const int sequencias_totais = 5;  // Número total de fases
int sequencia_atual = 0;  // Fase atual do jogo
int jogador_indice = 0;  // Posição atual do jogador na sequência

// Sequências predefinidas de acertos (0 = Botão A, 1 = Botão B)
int sequencias[5][7] = {
    {0, 1, 0, -1},         // Fase 1
    {1, 0, 1, 0, -1},      // Fase 2
    {0, 0, 1, 1, 0, -1},   // Fase 3
    {1, 1, 0, 0, 1, 0, -1},// Fase 4
    {0, 1, 1, 0, 1, 1, 0, -1}  // Fase 5
};

// Função de debounce para evitar múltiplos cliques acidentais
bool debounce(absolute_time_t *last_interrupt_time) {
    absolute_time_t current_time = get_absolute_time();  // Pega o tempo atual
    // Verifica se o tempo decorrido desde a última interrupção é maior que o delay de debounce
    if (absolute_time_diff_us(*last_interrupt_time, current_time) / 1000 < debounce_delay_ms) {
        return false;  // Se for menor, ignora o clique
    }
    *last_interrupt_time = current_time;  // Atualiza o tempo da última interrupção
    return true;  // Clique válido
}

// Função para configurar um pixel no LED WS2812
void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);  // Envia a cor para o LED
}

// Função que exibe a sequência de LEDs e caracteres no display
void exibir_sequencia() {
    // Itera pela sequência da fase atual
    for (int i = 0; sequencias[sequencia_atual][i] != -1; i++) {
        ssd1306_fill(&ssd, false);  // Limpa o display
        if (sequencias[sequencia_atual][i] == 0) {  // Botão A
            ssd1306_draw_string(&ssd, "A", 10, 30);  // Mostra 'A' no display
            set_pixel(0, 0x00FF00);  // LED verde
        } else {  // Botão B
            ssd1306_draw_string(&ssd, "B", 10, 30);  // Mostra 'B' no display
            set_pixel(0, 0x0000FF);  // LED azul
        }
        ssd1306_send_data(&ssd);  // Atualiza o display
        sleep_ms(1000);  // Espera 1 segundo
        set_pixel(0, 0x000000);  // Desliga o LED
        sleep_ms(500);  // Espera 0,5 segundo
    }
    ssd1306_fill(&ssd, false);  // Limpa o display no fim
    ssd1306_send_data(&ssd);
    jogador_indice = 0;  // Reinicia o índice do jogador
}

// Verifica se o jogador acertou a sequência
void verificar_acerto() {
    // Se o jogador completou a sequência atual corretamente
    if (sequencias[sequencia_atual][jogador_indice] == -1) {
        printf("Acertou! Passando para a próxima fase...\n");
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "Acertou!", 10, 30);  // Mostra 'Acertou!' no display
        ssd1306_send_data(&ssd);
        sleep_ms(2000);  // Pausa para feedback
        jogador_indice = 0;  // Reinicia o índice do jogador
        if (sequencia_atual < sequencias_totais - 1) {  // Avança para a próxima fase
            sequencia_atual++;
        } else {  // Se completou todas as fases, reinicia o jogo
            printf("Jogo concluído! Reiniciando...\n");
            sequencia_atual = 0;
        }
        exibir_sequencia();  // Mostra a nova sequência
    }
}

// Função de callback das interrupções dos botões
void gpio_callback(uint gpio, uint32_t events) {
    // Verifica se o botão A foi pressionado
    if (gpio == BOTAO_A && debounce(&last_interrupt_time_a)) {
        if (sequencias[sequencia_atual][jogador_indice] == 0) {  // Jogador pressionou o botão correto
            jogador_indice++;  // Avança para a próxima posição na sequência
            verificar_acerto();  // Verifica se a sequência foi completada
        } else {  // Jogador errou
            printf("Errou! Reiniciando fase %d.\n", sequencia_atual + 1);
            jogador_indice = 0;  // Reinicia o índice do jogador
            exibir_sequencia();  // Mostra a sequência novamente
        }
    }
    // Verifica se o botão B foi pressionado
    if (gpio == BOTAO_B && debounce(&last_interrupt_time_b)) {
        if (sequencias[sequencia_atual][jogador_indice] == 1) {  // Jogador pressionou o botão correto
            jogador_indice++;
            verificar_acerto();
        } else {  // Jogador errou
            printf("Errou! Reiniciando fase %d.\n", sequencia_atual + 1);
            jogador_indice = 0;
            exibir_sequencia();
        }
    }
}

int main() {
    stdio_init_all();  // Inicializa a comunicação serial
    i2c_init(I2C_PORT, 400 * 1000);  // Inicializa o barramento I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Define os pinos SDA e SCL
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);  // Ativa pull-up nos pinos I2C
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT);  // Inicializa o display OLED
    ssd1306_config(&ssd);  // Configura o display
    ssd1306_send_data(&ssd);  // Envia dados para o display
    sm = pio_claim_unused_sm(pio, true);  // Aloca uma máquina de estado no PIO
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);  // Inicializa os LEDs WS2812
    // Inicializa e configura os botões
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    // Habilita interrupções para os botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    exibir_sequencia();  // Exibe a sequência inicial do jogo
    while (true) {
        sleep_ms(100);
    }
}
