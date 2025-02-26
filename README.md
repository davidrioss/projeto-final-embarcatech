# InteliLixo: Sistema de Gerenciamento Inteligente de Coleta de Resíduos

## Descrição do Projeto
O **InteliLixo** é um sistema de gerenciamento inteligente de coleta de resíduos que utiliza um protótipo baseado no **Raspberry Pi Pico W** para simular uma lixeira inteligente. O sistema monitora o nível de resíduos na lixeira, fornece feedback visual através de uma matriz de LEDs e um display OLED, e envia dados para uma central de coleta, permitindo a otimização das rotas de coleta.

Este projeto foi desenvolvido para ser compilado e emulado na **Raspberry Pi Pico W** presente na placa **BitDogLab**, utilizando a extensão do Raspberry Pi Pico para **Visual Studio Code (VSCode)**.

---

## Funcionalidades
- **Simulação do Sensor Ultrassônico**: Utiliza o joystick da BitDogLab para simular a medição do nível de resíduos.
- **Feedback Visual**:
  - Matriz de LEDs 5x5: Representa o nível de resíduos de forma gráfica.
  - LED RGB: Indica o status da lixeira (Verde: <50%, Amarelo: 50-75%, Vermelho: >75%).
  - Display OLED: Exibe mensagens de status que representa a decisão da central (ex.: "NÃO COLETAR", "COLETAR TALVEZ", "COLETAR").
- **Comunicação com a Central**: Envia dados simulados (nível de resíduos e timestamp) para a central via monitor serial.
- **Botão de Reset**: Coloca a placa em modo de boot ao ser pressionado.

---

## Estrutura do Projeto
O diretório do projeto contém os seguintes arquivos:
- **projeto_final.c**: Código principal do projeto.
- **CMakeLists.txt**: Configuração do projeto para compilação com CMake.
- **pico_sdk_import.cmake**: Importação do SDK do Raspberry Pi Pico.
- **projeto_final.pio**: Configuração do PIO para controle da matriz de LEDs.
- **ssd1306.c** e **ssd1306.h**: Driver para o display OLED.
- **ws2812.pio**: Configuração do PIO para controle dos LEDs WS2812B.
- **fonte.h**: fonte usada na exibição de mensagem no display.

---

## Requisitos
- **Hardware**:
  - Raspberry Pi Pico W.
  - Placa BitDogLab (para simulação com joystick e LEDs).

- **Software**:
  - Visual Studio Code com extensão **Raspberry Pi Pico**.
  - SDK do Raspberry Pi Pico.
  - Ferramentas de compilação (CMake, GCC, etc.).

---

## Compilação e Execução
1. **Configuração do Ambiente**:
   - Instale o SDK do Raspberry Pi Pico e configure o VSCode com a extensão do Pico.
   - Clone este repositório ou copie os arquivos para o seu workspace.

2. **Compilação**:
   - Abra o projeto no VSCode.
   - Configure o `CMakeLists.txt` para o seu ambiente.
   - Compile o projeto usando a opção **Build** no VSCode.

3. **Upload para a Placa**:
   - Conecte a Raspberry Pi Pico W ao computador no modo de bootloader.
   - Copie o arquivo `.uf2` gerado para a placa.

4. **Execução**:
   - Conecte a placa BitDogLab e ligue o sistema.
   - Acompanhe os dados no monitor serial e observe o comportamento do sistema.

## Vídeo de demonstração

- Link para o vídeo de demonstração: <https://youtu.be/y5NXZRv1gok>

---

## Exemplo de Saída
O sistema envia dados no formato JSON para o monitor serial, como no exemplo abaixo:
```json
{"id_lixeira": "001", "nivel": 65%, "timestamp": "2025-02-25T14:30:00Z"}
