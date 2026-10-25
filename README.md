# ♻️ Filament Maker: Protótipo de Pultrusora PET para Impressão 3D

[![Arduino Uno](https://img.shields.io/badge/Microcontroller-Arduino%20Uno%20R3-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language: C++](https://img.shields.io/badge/Firmware-C%2B%2B%20%2F%20Wiring-blue?logo=c%2B%2B&logoColor=white)](./firmware/filament_maker.ino)
[![Thermal Control: PID](https://img.shields.io/badge/Thermal%20Control-PID%20Closed--Loop-orange)](./docs/evolucao-arquitetura.md)
[![Recycling: PET Bottles](https://img.shields.io/badge/Recycling-PET%20Bottles-green)](./docs/ensaios-e-testes-praticos.md)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

O **Filament Maker** é um protótipo aberto de reciclagem termomecânica com o objetivo de transformar fitas cortadas de garrafas de **PET (Polietileno Tereftalato)** em material para impressão 3D. O projeto ainda não apresenta medições registradas que comprovem o diâmetro ou as propriedades do filamento produzido.

O sistema combina um termistor NTC de 100 kΩ, controle PID por PWM, um módulo MOSFET, um bloco aquecedor adaptado e tração manualmente ajustável com motor de passo NEMA 17 e driver A4988. O firmware desliga o aquecedor quando detecta falha na leitura do sensor ou temperatura acima de 265 °C.

---

## 📑 Sumário

- [Visão Geral: O que é e Para que Serve](#-visão-geral-o-que-é-e-para-que-serve)
- [Princípio de Funcionamento: O Processo de Pultrusão](#-princípio-de-funcionamento-o-processo-de-pultrusão)
- [Como Foi Feito: Arquitetura de Hardware e Eletrônica](#-como-foi-feito-arquitetura-de-hardware-e-eletrônica)
  - [Lista de Componentes Principais](#lista-de-componentes-principais)
  - [Esquema de Conexões (Texto Descritivo e Detalhado)](#esquema-de-conexões-texto-descritivo-e-detalhado)
  - [Diagrama Esquemático de Ligações](#diagrama-esquemático-de-ligações)
  - [Tabela de Pinagem do Arduino](#tabela-de-pinagem-do-arduino)
- [Firmware e Estratégia de Controle](#-firmware-e-estratégia-de-controle)
  - [Temporização do firmware](#temporização-do-firmware)
  - [Controle e Limites de Temperatura](#controle-e-limites-de-temperatura)
  - [Acionamento manual da tração](#acionamento-manual-da-tração)
  - [Calibração de Corrente do Driver A4988 ($V_{ref}$)](#calibração-de-corrente-do-driver-a4988-v_ref)
- [Arquitetura Atual e Decisões de Implementação](./docs/evolucao-arquitetura.md)
- [Configurações de Fatiador Experimentadas](#-configurações-de-fatiador-experimentadas)
- [Como Reproduzir e Operar](#-como-reproduzir-e-operar)
- [Créditos e Referências Bibliográficas (ABNT)](#-créditos-e-referências-bibliográficas-abnt)
- [Licença](#-licença)

---

## 🎯 Visão Geral: O que é e Para que Serve

O projeto explora uma abordagem de reaproveitamento de embalagens PET para impressão 3D. Este repositório documenta a arquitetura do protótipo, o firmware e algumas configurações de fatiador experimentadas. Não há resultados medidos registrados sobre propriedades mecânicas, térmicas, diâmetro, produtividade ou impacto ambiental do material produzido.

---

## 🔬 Princípio de Funcionamento: O Processo de Pultrusão

O sistema adota o método de **pultrusão térmica** contínua:

```text
 ┌─────────────────────┐       ┌─────────────────────┐       ┌─────────────────────┐       ┌─────────────────────┐
 │ 1. PREPARAÇÃO       │ ───>  │ 2. FILETAMENTO      │ ───>  │ 3. AQUECIMENTO      │ ───>  │ 4. TRAÇÃO           │
 │ Limpeza e secagem   │       │ Corte de fita PET   │       │ Setpoint do bloco   │       │ NEMA 17 manual      │
 └─────────────────────┘       └─────────────────────┘       └─────────────────────┘       └─────────────────────┘
```

1. **Preparação da garrafa**:
   - Remova rótulos e resíduos e lave e seque a garrafa antes do corte.
   - Este repositório não documenta um método validado para remover vincos. Não aqueça garrafas pressurizadas nem use chama para esse fim.
2. **Filetamento Contínuo**:
   - O fundo da garrafa é recortado e a borda inicial recebe um corte chanfrado em triângulo.
   - A garrafa pode ser cortada em fita com um cortador apropriado. Não há medidas registradas que estabeleçam uma largura de corte ideal por tipo de garrafa.
3. **Pultrusão Térmica no Hotend Creality Mk8 Adaptado**:
   - O protótipo usa um bloco aquecedor adaptado, controlado pelo firmware. A conformação da fita é o objetivo do processo; o repositório não registra medições que comprovem diâmetro ou geometria do material produzido.
4. **Tração Mecânica e Bobinamento**:
   - A ponta inicial é transpassada a quente com auxílio de alicate/pinça e presa nas engrenagens de tração (adaptadas do projeto [Petalot](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357)).
   - O operador ajusta a tração pelo potenciômetro. O firmware calcula a velocidade considerando 1 a 30 RPM, mas não há medições registradas da velocidade mecânica real.

---

## 🛠️ Como Foi Feito: Arquitetura de Hardware e Eletrônica

### Lista de Componentes Principais

| Categoria | Componente | Especificação Técnica | Função no Sistema |
| :--- | :--- | :--- | :--- |
| **Controlador** | Arduino Uno R3 | Microcontrolador ATmega328P (16 MHz, ADC 10 bits) | Leitura do sensor, controle PID e geração de passos |
| **Sensor Térmico** | Termistor NTC 100k | Tipo gotinha de vidro ($\beta_{25/50} = 3950$, $R_{25} = 100\text{ k}\Omega$) | Leitura de temperatura do bloco aquecedor |
| **Resistor Divisor** | Resistor fixo | 10 kΩ nominal no firmware; atualizar `R_SERIE` para o valor real medido | Divisor de tensão para conversão analógica em `A0` |
| **Módulo MOSFET** | MOSFET de Potência HA210N06 | Módulo de acionamento para extrusoras 3D (baixo $R_{DS(on)}$) | Comutação PWM do cartucho aquecedor |
| **Resistores do Gate** | Resistores de Filme | $220\,\Omega$ em série e $10\text{ k}\Omega$ pull-down | Limitação de corrente transitória e nível baixo no Gate quando a entrada não é acionada |
| **Bloco Térmico** | Hotend Creality Mk8 Completo | Modelo Ender 3 / CR10 com abertura cônica alargada com broca | Aquecimento e conformação da fita em filamento |
| **Elemento Aquecedor** | Cartucho Cerâmico 12V | 40W montado no bloco aquecedor Mk8 | Fonte de calor controlada por PWM |
| **Interface Visual** | Display TM1637 | 4 dígitos e 7 segmentos vermelhos | Exibição da temperatura em tempo real com 1 casa decimal |
| **Motor de Tração** | Motor de Passo NEMA 17 | Modelo 17HS4401 (torque de 4.0 kgf.cm) | Puxamento do filamento na saída do bico |
| **Driver de Motor** | Módulo Driver A4988 | O cálculo de velocidade do firmware assume 2 micropassos por passo completo | A configuração física dos pinos de micropasso não é controlada pelo firmware |
| **Filtro de Alimentação**| Capacitor Eletrolítico | $100\ \mu\text{F} / 35\text{V}$ soldado entre `VMOT` e `GND` | Proteção do driver contra picos de tensão indutiva |
| **Ajuste de Velocidade**| Potenciômetro Rotativo | Linear de $10\text{ k}\Omega$ conectado ao canal `A1` | Acionamento manual e regulagem de velocidade |
| **Cortador de Fita** | Simple PET Bottle Cutter | Modelo impresso com lâmina de estilete | Corte de tiras de garrafa PET |
| **Fonte Externa** | Fonte Chaveada 12V DC | 12V / 10A (com GND unificado ao Arduino) | Alimentação de alta corrente do aquecedor e motor |

---

### Diagrama Esquemático de Ligações

```text
========================================================================================
 1. SENSOR DE TEMPERATURA (Divisor de Tensão NTC 100k)
========================================================================================
 [ +5V Arduino ] ───[ Resistor 10k ]───┬───[ NTC 100k ]─── [ GND Arduino ]
                                                             │
                                                             └───> ( Pino Analógico A0 )

========================================================================================
 2. ACIONAMENTO DE POTÊNCIA (MOSFET HA210N06)
========================================================================================
  [ Pino 9 Arduino ] ───[ Resistor 1k ]───> [ BASE (Pino 2) ]
                                                │
        [ +12V da Fonte  ] ───[ Resistor 10k]───┬───> [ COLETOR (Pino 1) ]
                                        │
                                        └───> [ Módulo MOSFET: Pino SIG ]

 [ EMISSOR (Pino 3) ] ───┐
 [ MOSFET: Pino GND ] ───┼───> [ NEGATIVO (-) DA FONTE 12V ] ───> [ GND do Arduino ]
                                             
                                             
 [ +12V Fonte  ] ────────────────────────────> [ Módulo MOSFET: Power In + ]
 [ -12V Fonte  ] ────────────────────────────> [ Módulo MOSFET: Power In - ] ─── [ GND Comum ]
 [ Módulo MOSFET: Bornes Saída ] ────────────> [ Cartucho Aquecedor (40W) ]
========================================================================================
 3. DISPLAY TM1637 (4 Dígitos, 7 Segmentos)
========================================================================================
 [ +5V Arduino ] ──────────────────────────────────────────> [ VCC ]
 [ GND Arduino ] ──────────────────────────────────────────> [ GND ]
 [ Pino Digital 2 ] ───────────────────────────────────────> [ CLK ]
 [ Pino Digital 3 ] ───────────────────────────────────────> [ DIO ]

========================================================================================
 4. MECANISMO DE TRAÇÃO (Driver A4988 + Motor NEMA 17)
========================================================================================
 Sinais Lógicos:
  [ Pino Digital 4 ] ──────────────────────────────────────> [ A4988: STEP ]
  [ Pino Digital 5 ] ──────────────────────────────────────> [ A4988: DIR ]
  [ Pino Digital 6 ] ──────────────────────────────────────> [ A4988: ENABLE ] (LOW = Ativo)
  [ +5V Arduino ] ─────────────────────────────────────────> [ A4988: VDD ]
  [ GND Arduino ] ─────────────────────────────────────────> [ A4988: GND Lógico ]

  [ A4988: SLEEP ] ─── jumpeado fisicamente ───────────────> [ A4988: RESET ]
  [ A4988: MS1, MS2, MS3 ] ─── +5V (HIGH),	GND (LOW),	GND (LOW)

 Alimentação e Bobinas do Motor:
  [ +12V Fonte ] ───────────────────┬──────────────────────> [ A4988: VMOT ]
                                    │ [Capacitor Eletrolítico]
                                 [ 100 µF / 35V ]
                                    │
  [ GND Fonte / Comum ] ────────────┴──────────────────────> [ A4988: GND Potência ]

  [ A4988: 1A e 1B ] ──────────────────────────────────────> [ Bobina A do NEMA 17 ]
  [ A4988: 2A e 2B ] ──────────────────────────────────────> [ Bobina B do NEMA 17 ]

========================================================================================
 5. CONTROLE MANUAL DE VELOCIDADE
========================================================================================
  [ +5V Arduino ] ─────────────────────────────────────────> [ Potenciômetro: Terminal 1 ]
  [ Pino Analógico A1 ] ───────────────────────────────────> [ Potenciômetro: Cursor Central ]
  [ GND Arduino ] ─────────────────────────────────────────> [ Potenciômetro: Terminal 3 ]
========================================================================================
```

---

### Tabela de Pinagem do Arduino

| Pino Arduino | Direção | Tipo de Sinal | Destino / Função |
| :---: | :---: | :---: | :--- |
| `A0` | Entrada | Analógico (0–5V) | Leitura analógica do divisor de tensão do termistor NTC 100k |
| `A1` | Entrada | Analógico (0–5V) | Leitura do potenciômetro para acionamento manual e controle de RPM |
| `D2` | Saída | Digital | Sinal de clock serial (`CLK`) para o display TM1637 |
| `D3` | I/O | Digital | Barramento de dados (`DIO`) para o display TM1637 |
| `D4` | Saída | Digital | Trem de pulsos de passo (`STEP`) para o driver A4988 |
| `D5` | Saída | Digital | Sinal de sentido de rotação (`DIR`) do carretel de tração |
| `D6` | Saída | Digital | Habilitação do motor (`ENABLE`): nível `LOW` quando a leitura do potenciômetro é igual ou superior a 25 |
| `D9` | Saída | Digital PWM | Sinal de controle modulado para o Gate do MOSFET HA210N06 |

---

## 💻 Firmware e Estratégia de Controle

O código-fonte oficial encontra-se em [`firmware/filament_maker.ino`](./firmware/filament_maker.ino).

### Acionamento manual da tração
A velocidade é ajustada manualmente por um potenciômetro:
1. Com o potenciômetro no mínimo (`valorPotenciometro < 25`), o pino `ENABLE` vai para `HIGH`, desenergizando as bobinas do motor NEMA 17.
2. Com o bloco aquecido, a ponta da fita é conduzida pelo bico com ferramenta apropriada. Evite contato com o bloco e o bico quentes.
3. Com o motor desenergizado, a ponta conformada é engatada e fixada com firmeza nas engrenagens de tração.
4. O operador gira o potenciômetro, acionando o motor no momento exato e regulando visualmente a tração contínua.

---

## 📈 Evolução das Decisões de Arquitetura

O projeto passou por sucessivas iterações até atingir robustez industrial:

1. **Abandono do Termostato Comercial W1209**: Descartado por limitações de escala (< 110 °C de fábrica) e histerese oscilatória de relé mecânico (±15 °C), sendo substituído pelo Arduino Uno com controle de alta frequência.
2. **Implementação do MOSFET HA210N06**: Adoção do módulo dedicado com resistor de proteção de Gate ($220\,\Omega$) e resistor de pull-down ($10\text{ k}\Omega$), permitindo comutação silenciosa e segura sem superaquecimento.
3. **Hotend Creality Mk8 com Entrada Cônica**: Adoção do bico/bloco Mk8 com usinagem cônica por broca, viabilizando o afunilamento gradual da fita plana em tubo fechado.
4. **Setpoint em 250 °C para Compensação de Carga Térmica**: Definição empírica de 250 °C no firmware para garantir que a temperatura efetiva durante a passagem contínua da fita permaneça entre 235 °C e 245 °C.
5. **Tração Manual via Potenciômetro**: Substituição do sistema automático com fio guia pelo acionamento manual direto via potenciômetro para viabilizar a fixação segura da ponta da fita nas engrenagens.

> 📖 **Consulte o relatório detalhado em**: [`docs/evolucao-arquitetura.md`](./docs/evolucao-arquitetura.md).

---

## 🧪 Ensaios Práticos, Geometria e Diagnóstico de Bancada

### Natureza Oca do Filamento e Parâmetros de Fatiamento (Calibração de Fluxo)

O filamento gerado por pultrusão térmica **é oco** (possui formato tubular com costura longitudinal fechada em "O", contendo ar no núcleo e espessura de parede correspondente à espessura original da garrafa PET).

Como consequência física, um filamento de diâmetro externo de 1.75 mm possui menor massa linear de plástico por milímetro do que um filamento maciço tradicional.

#### Padrões de Calibração de Fatiamento (OrcaSlicer / Bambu Studio / Cura)
Para compensar a cavidade oca e obter peças sólidas e resistentes sem subextrusão, devem ser adotados os seguintes parâmetros (validados em ensaios com o modelo *CaliCat*):

| Parâmetro de Fatiamento | Valor Calibrado | Justificativa Técnica |
| :--- | :---: | :--- |
| **Multiplicador de Fluxo (Flow Ratio)** | **$1.15$ a $1.35$ ($115\%$ a $135\%$)** | Compensa a ausência de núcleo sólido, garantindo que o volume real extrudado preencha 100% das linhas de impressão. |
| **Diâmetro Efetivo do Filamento** | **$1.30\text{ mm}$ a $1.35\text{ mm}$** | Alternativa de cálculo direto: configurar o diâmetro equivalente de massa no fatiador para compensar a seção vazada de 1.75 mm. |
| **Temperatura do Bico (Nozzle)** | **$250\ ^\circ\text{C}$ (1ª camada) / $255\ ^\circ\text{C}$** | Garante fluidez térmica ideal e forte adesão entre camadas para o polímero PET. |
| **Temperatura da Mesa (Bed)** | **$70\ ^\circ\text{C}$ a $80\ ^\circ\text{C}$** | Garante aderência consistente em placa PEI texturizada sem empenamento (*warp*). |
| **Distância de Retração** | **$0.4\text{ mm}$ (Direct Drive)** | Retração curta para evitar deformação ou engasgamento do filamento tubular oco na zona fria do hotend. |
| **Velocidade Volumétrica Máxima** | **$6.0\text{ mm}^3/\text{s}$** | Taxa controlada para garantir tempo de residência e fusão completa do filamento de garrafa. |

---

### Geometria Transversal: Formato em "O" vs. Perfil em "C"

```text
       Perfil Aberto ("C")                     Perfil Fechado ("O")
     [ Incompatível / Falha ]                  [ Nominal / Perfeito ]
         ┌─────────┐                                 ┌─────────┐
       ┌─┘         └─┐                             ┌─┘         └─┐
      │               │                           │       █       │
      │   (Abertura)  │                           │   (Soldado)   │
       └─┐         ┌─┘                             └─┐         ┌─┘
         └─────────┘                                 └─────────┘
   - Colapso sob tração                      - Tubo cilíndrico homogêneo
   - Patinamento nas engrenagens             - Diâmetro externo 1.75 mm
   - Rejeitado pela Bambu Lab A1             - Compatível com Bambu Lab A1
```

### Compatibilidade com a Bambu Lab A1
* Se o filamento for conformado com seção aberta em formato de **"C"**, a extrusora de acionamento direto (*Direct Drive*) da **Bambu Lab A1** **não consegue tracionar o filamento**. As engrenagens duplas de alta pressão comprimem o perfil aberto, provocando colapso da fita, patinamento e perda de alimentação.
* Para funcionamento perfeito na Bambu Lab A1 e outras impressoras modernas, a fita precisa obrigatoriamente fundir suas bordas e fechar o perfil em **"O" completo**.

---

### Relação de Corte vs. Tipo de Garrafa

A largura do filete deve compensar a espessura da parede da garrafa para atingir a seção transversal necessária:

| Tipo de Garrafa | Espessura de Parede ($e$) | Largura do Corte | Comportamento na Pultrusão e Fechamento | Desempenho do Motor NEMA 17 |
| :--- | :---: | :---: | :--- | :--- |
| **Refrigerante (Coca-Cola / Guaraná)** | $\approx 0.30\text{ mm}$ | **$7.0\text{ mm}$** | **Perfeito**. Fecha em formato "O" completo e tubular. | Tração estável, motor frio e sem perda de passos. |
| **Água Mineral Lisa (Fina)** | $\approx 0.15\text{ mm} - 0.20\text{ mm}$ | **$7.0\text{ mm}$** | **Insuficiente**. Falta massa; forma perfil em "C" que falha na Bambu Lab A1. | Sem esforço mecânico, mas filamento inutilizável. |
| **Água Mineral Lisa (Fina)** | $\approx 0.15\text{ mm} - 0.20\text{ mm}$ | **$10.0\text{ mm} - 12.0\text{ mm}$** | **Adequado**. Compensa a espessura fina e fecha o tubo circular. | Tração suave e diâmetro regular. |
| **Água com Gás (Espessa)** | $\approx 0.35\text{ mm} - 0.45\text{ mm}$ | **$7.0\text{ mm}$** | **Excessivo**. Fita espessa acumula massa e emperra no bico. | **Falha crítica**: Motor **perde passos** por sobretorque. |
| **Água com Gás (Espessa)** | $\approx 0.35\text{ mm} - 0.45\text{ mm}$ | **$5.0\text{ mm} - 5.5\text{ mm}$** | **Adequado**. Alivia o atrito e permite conformação fluida. | Tração restabelecida dentro dos parâmetros nominais. |

---

### Adaptações Mecânicas e Modelos 3D Utilizados

1. **Engrenagens de Tração do Projeto Petalot**:
   - Extraídas e adaptadas a partir do modelo [Petalot (MakerWorld)](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357).
   - **Reforço com Barras Fixas na Base de Madeira**: Para montar as engrenagens sobre bancada plana de madeira (sem o chassi original), **adicionaram-se duas barras fixas transversais** interligando diretamente os suportes das engrenagens, eliminando torção lateral e flexão sob esforço de tração.
2. **Cortador de Garrafa PET**:
   - Utilizado o modelo [Simple PET bottle cutter (MakerWorld)](https://makerworld.com/pt/models/98441-simple-pet-bottle-cutter#profileId-111492) com lâmina de estilete afiada para corte helicoidal contínuo e consistente.
3. **Bico de Pressurização (*Crushed Bottle Fixer*)**:
   - Utilização do bico rosqueável [Crushed Bottle Fixer (MakerWorld)](https://makerworld.com/pt/models/2620743-crushed-bottle-fixer#profileId-2892599) para pressurizar garrafas amassadas antes da aplicação de calor externo para alisamento.

> 📖 **Consulte o relatório completo de testes em**: [`docs/ensaios-e-testes-praticos.md`](./docs/ensaios-e-testes-praticos.md).

---

## 🚀 Como Reproduzir e Operar

### 1. Pré-Requisitos e Bibliotecas Arduino
Instale via Gerenciador de Bibliotecas da Arduino IDE:
- `PID` (por Brett Beauregard - versão `PID_v1`)
- `TM1637Display` (por Avishay Orpaz)

### 2. Configuração do Firmware
Abra [`firmware/filament_maker.ino`](./firmware/filament_maker.ino):
1. Verifique a constante `R_SERIE` com o valor real medido do seu resistor fixo no multímetro:
   ```cpp
   const float R_SERIE = 10000.0; // Atualize com o valor medido em ohms
   ```
2. Carregue o firmware na placa **Arduino Uno**.

### 3. Procedimento Operacional Passo a Passo
1. **Ligar o Aquecimento**: Conecte a fonte de 12 V. O firmware controla a leitura do sensor no bloco em direção ao setpoint de **250 °C**. Não há registro do tempo de aquecimento nem de medição independente da temperatura da fita.
2. **Inserir a Fita Manualmente**: Com o potenciômetro no zero (motor parado), passe a ponta chanfrada da fita pelo bico cônico quente do hotend Mk8.
3. **Puxar e Prender**: Puxe a ponta que sai do bico com o auxílio de um alicate ou pinça até alcançar as engrenagens de tração e fixe-a firmemente nos dentes de arraste.
4. **Acionar a Tração**: Gire o potenciômetro. O firmware mapeia a entrada para uma faixa calculada de 1 a 30 RPM; a velocidade real depende da configuração do A4988 e da mecânica, e não há medição registrada.

---

## 📚 Créditos e Referências Bibliográficas (ABNT)

* **LEMOS, Taís Pereira de.** *Avaliação do uso de poli(tereftalato de etileno) pós-consumo como filamento para impressão 3D.* Dissertação (Mestrado em Engenharia Metalúrgica e de Materiais) – COPPE/UFRJ, 2018.
* **SANTOS, João Victor dos et al.** *REPET: Reaproveitador de Polietileno Tereftalato.* TCC (Técnico em Mecatrônica) – ETEC Júlio de Mesquita / CPS, 2023.
* **SOMENZI, André; FILIPETTO, Leane Maria.** *Utilização de filamento reciclado de garrafas PET em impressão 3D: uma aplicação do Design Science Research.* IFRS, 2023.
* **FERNANDES, Sarah Even O. B.** *Desenvolvimento de uma extrusora de filamento PET para impressão 3D.* II CONASUST, 2024.
* **SILVA, Deley A. N. da et al.** *Construindo uma filamentadora semi automatizada.* Concurso de Ideias - SBC, 2025.
* **PLASTIC ODYSSEY.** *3D printing from PET bottles: a step-by-step tutorial.* Recycling Academy, 2023.
* **MODELOS 3D E PROJETOS DA COMUNIDADE**:
  - [Petalot: Plastic Bottles into 3D Filament (MakerWorld)](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357) – Referência para as engrenagens de tração.
  - [Simple PET Bottle Cutter (MakerWorld)](https://makerworld.com/pt/models/98441-simple-pet-bottle-cutter#profileId-111492) – Gabarito cortador de tiras.
  - [PETamentor 2](https://petamentor2.com/), [Recreator 3D MK5/MK6](https://recreator3d.com/) e [Polyformer](https://github.com/Reiten966/Polyformer).
* **COMUNIDADE MAKER**: *Ali3d Brasil*, *Eng. Felipe*, *Everton*, *Tutoriais 3D*, *MakerHero* e *Indústria Maker 4.0*.

> 📖 **Consulte a compilação bibliográfica completa e padronizada em**: [`docs/referencias-bibliograficas.md`](./docs/referencias-bibliograficas.md).

---

## 📜 Licença

Distribuído sob a licença **MIT**. Consulte `LICENSE` para mais detalhes. Sinta-se livre para reproduzir, aprimorar e compartilhar para impulsionar a manufatura sustentável e a reciclagem aberta.
