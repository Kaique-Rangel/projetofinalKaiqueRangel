# EmbarcaTech

### PROJETO FINAL 26/02/2025

#### Grupo 4 - Prof. Clea

##### Autor:
* Kaique Rangel Da Silva

## Instruções de Compilação

Certifique-se de que seu ambiente está configurado corretamente:

1. Instale e configure o **Pico SDK**.
2. Tenha o **VSCode** instalado com as extensões adequadas, além do **CMake** e **Make**.
3. Clone o repositório e abra a pasta do projeto.
4. A extensão **Pi Pico** criará automaticamente a pasta `build`.
5. Clique em **Compile** na barra inferior direita (ao lado de **RUN | PICO SDK**).
6. Clique em **RUN**.
10. **Botão A**: Interação quando aparece a letra A.
11. **Botão B**: Interação quando aparece a letra B.

## Descrição do Projeto

O projeto utiliza um **Raspberry Pi Pico** para interação com um **display SSD1306**, LEDs comuns e LEDs **WS2812**. Além disso, há interação via **Serial Monitor** para exibição de caracteres para serem digitados.

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

#### 5. font.h
* Foram criados caracteres minúsculos para a font.h.

## Organização do Código

- **Interrupções GPIO**: Manipulação dos botões e LEDs.
- **Debouncing**: Implementado para evitar múltiplas leituras falsas.
- **Controle WS2812**: Exibição de números na matriz de LEDs.
- **Display SSD1306**: Uso de **I2C** para exibição de caracteres digitados.




