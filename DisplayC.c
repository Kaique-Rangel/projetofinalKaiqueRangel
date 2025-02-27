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
#define ENDERECO 0x3C

#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW false

#define BOTAO_A 5
#define BOTAO_B 6
#define LED_VERDE 11
#define LED_AZUL 12

PIO pio = pio0;
uint sm = 0;
ssd1306_t ssd;
absolute_time_t last_interrupt_time_a = {0};
absolute_time_t last_interrupt_time_b = {0};
const uint32_t debounce_delay_ms = 200;

const int sequencias_totais = 5;
int sequencia_atual = 0;
int jogador_indice = 0;

// Sequências predefinidas
int sequencias[5][8] = {
    {0, 1, 0, -1},      // Fase 1
    {1, 0, 1, 0, -1},   // Fase 2
    {0, 0, 1, 1, 0, -1},// Fase 3
    {1, 1, 0, 0, 1, 0, -1}, // Fase 4
    {0, 1, 1, 0, 1, 1, 0, -1}  // Fase 5
};

bool debounce(absolute_time_t *last_interrupt_time) {
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(*last_interrupt_time, current_time) / 1000 < debounce_delay_ms) {
        return false;
    }
    *last_interrupt_time = current_time;
    return true;
}

void set_pixel(uint index, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

void exibir_sequencia() {
    jogador_indice = 0; // Reinicia o progresso do jogador
    printf("Exibindo sequência da fase %d...\n", sequencia_atual + 1);

    for (int i = 0; sequencias[sequencia_atual][i] != -1; i++) {
        ssd1306_fill(&ssd, false);
        if (sequencias[sequencia_atual][i] == 0) {
            ssd1306_draw_string(&ssd, "A", 10, 30);
            set_pixel(0, 0x00FF00); // Verde
        } else {
            ssd1306_draw_string(&ssd, "B", 10, 30);
            set_pixel(0, 0x0000FF); // Azul
        }
        ssd1306_send_data(&ssd);
        sleep_ms(1000);
        set_pixel(0, 0x000000);
        sleep_ms(500);
    }

    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    printf("Sequência exibida. Aguardando entradas do jogador...\n");
}

void verificar_acerto() {
    if (sequencias[sequencia_atual][jogador_indice] == -1) {
        printf("Acertou a sequência completa! Avançando para a próxima fase...\n");

        // Limpa o display antes de desenhar "Acertou!"
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "Acertou!", 10, 30);
        ssd1306_send_data(&ssd);

        // Pausa para o jogador ver a mensagem "Acertou!"
        sleep_ms(2000);

        // Avançar para a próxima sequência
        if (sequencia_atual < sequencias_totais - 1) {
            sequencia_atual++;  // Avança para a próxima fase
            printf("Próxima fase: %d\n", sequencia_atual + 1);
        } else {
            // Se todas as fases foram concluídas, reinicia do início
            printf("Jogo concluído! Reiniciando do início...\n");
            sequencia_atual = 0;
        }

        jogador_indice = 0;  // Reinicia o índice do jogador

        exibir_sequencia();  // Exibe a próxima sequência
    }
}

void reiniciar_fase() {
    // Função para reiniciar a fase atual após erro
    printf("Errou! Reiniciando fase %d.\n", sequencia_atual + 1);

    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "Reiniciando...", 10, 30);
    ssd1306_send_data(&ssd);
    sleep_ms(2000);

    jogador_indice = 0;
    exibir_sequencia();
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO_A && debounce(&last_interrupt_time_a)) {
        printf("Botão A pressionado\n");
        if (sequencias[sequencia_atual][jogador_indice] == 0) {
            jogador_indice++;
            if (sequencias[sequencia_atual][jogador_indice] == -1) {
                verificar_acerto();
            }
        } else {
            reiniciar_fase();  // Reinicia a fase em caso de erro
        }
    }
    if (gpio == BOTAO_B && debounce(&last_interrupt_time_b)) {
        printf("Botão B pressionado\n");
        if (sequencias[sequencia_atual][jogador_indice] == 1) {
            jogador_indice++;
            if (sequencias[sequencia_atual][jogador_indice] == -1) {
                verificar_acerto();
            }
        } else {
            reiniciar_fase();  // Reinicia a fase em caso de erro
        }
    }
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    exibir_sequencia();

    while (true) {
        sleep_ms(100);
    }
}
