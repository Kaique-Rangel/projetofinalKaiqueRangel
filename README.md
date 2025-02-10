# EmbarcaTech

### TAREFA aula síncrona 03/02/2025

#### Grupo 4 - Prof. Clea

##### Autor:
* Kaique Rangel Da Silva

#### Vídeo de funcionamento

* Link: https://youtu.be/6_cGg9JLwdo

## Instruções de Compilação

Certifique-se de que seu ambiente está configurado corretamente:

1. Instale e configure o **Pico SDK**.
2. Tenha o **VSCode** instalado com as extensões adequadas, além do **CMake** e **Make**.
3. Clone o repositório e abra a pasta do projeto.
4. A extensão **Pi Pico** criará automaticamente a pasta `build`.
5. Clique em **Compile** na barra inferior direita (ao lado de **RUN | PICO SDK**).
6. Clique em **RUN**.
7. Abra o **SERIAL MONITOR** (próximo a **XRTOS**).
8. Clique em **Start Monitoring** e escreva caracteres.
9. Os caracteres digitados aparecerão no display da placa **BitDogLab**.
10. **Botão A**: Liga/desliga o **LED Verde** e exibe mensagem no display.
11. **Botão B**: Liga/desliga o **LED Azul** e exibe mensagem no display.
12. Se ambos os LEDs estiverem ligados, suas cores se misturam.

## Descrição do Projeto

O projeto utiliza um **Raspberry Pi Pico** para interação com um **display SSD1306**, LEDs comuns e LEDs **WS2812**. Além disso, há interação via **Serial Monitor** para exibição de caracteres digitados.

### Funcionalidades Principais

#### 1. Interrupções (IRQ)
* Os botões utilizam **interrupções** para controle do estado dos LEDs.

#### 2. Debouncing
* Implementação de **debounce via software** para evitar leituras incorretas dos botões.

#### 3. Controle de LEDs
* Utilização de **LEDs comuns** e **LEDs WS2812**.
* Exibição de números na **matriz 5x5** dos LEDs WS2812 quando um número é digitado via Serial Monitor.

#### 4. Uso do Display SSD1306 (128x64)
* Exibição de caracteres **maiúsculos e minúsculos**.
* Interface com **I2C**.

#### 5. Comunicação via UART
* Entrada de caracteres pelo **Serial Monitor do VSCode**.
* Exibição dos caracteres digitados no **display SSD1306**.
* Quando um número entre **0 e 9** for digitado, ele será exibido na **matriz WS2812**.

#### 6. font.h
* Foram criados caracteres minúsculos para a font.h.

## Organização do Código

- **Interrupções GPIO**: Manipulação dos botões e LEDs.
- **Debouncing**: Implementado para evitar múltiplas leituras falsas.
- **Controle WS2812**: Exibição de números na matriz de LEDs.
- **Display SSD1306**: Uso de **I2C** para exibição de caracteres digitados.
- **Comunicação Serial**: Interação via **UART**.




