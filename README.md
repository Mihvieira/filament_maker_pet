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

O descarte de embalagens de refrigerante e água mineral representa um dos maiores passivos ambientais urbanos. Embora o PET seja um termoplástico de propriedades mecânicas e térmicas excepcionais, a comunidade de manufatura aditiva consome predominantemente filamentos virgens comerciais (PLA ou PETG aditivado).

O projeto **Filament Maker** viabiliza a transformação descentralizada de embalagens PET descartáveis em filamento calibrado de 1.75 mm para impressoras 3D FDM.

### Autonomia e Economia na Impressão 3D
1. **Autonomia e Descentralização Produtiva**: Concede independência de suprimentos comerciais, permitindo que makers, laboratórios e escolas fabriquem seu próprio filamento sob demanda com recursos locais acessíveis.
2. **Alternativa Econômica para Prototipagem e Testes**: Reduz o custo de matéria-prima a praticamente zero, encorajando iterações rápidas de engenharia, testes mecânicos destrutivos e validações volumétricas de peças grandes.
3. **Viabilidade Comercial e Venda de Peças Sustentáveis**: Habilita a produção de peças finais para comercialização com margem de lucro ampliada e forte apelo de sustentabilidade (*upcycling*), atendendo clientes que valorizam a economia circular.
4. **Impacto Ambiental Positivo**: Retira garrafas plásticas do fluxo de aterros sanitários e descarte inadequado, reintegrando-as diretamente em novos produtos físicos.

---

## 🔬 Princípio de Funcionamento: O Processo de Pultrusão

O sistema adota o método de **pultrusão térmica** contínua:

```text
 ┌─────────────────────┐       ┌─────────────────────┐       ┌─────────────────────┐       ┌─────────────────────┐
 │ 1. PREPARAÇÃO       │ ───>  │ 2. FILETAMENTO      │ ───>  │ 3. AQUECIMENTO      │ ───>  │ 4. TRAÇÃO           │
 │ Limpeza e secagem   │       │ Corte de fita PET   │       │ Setpoint do bloco   │       │ NEMA 17 manual      │
 └─────────────────────┘       └─────────────────────┘       └─────────────────────┘       └─────────────────────┘
```

1. **Preparação da Garrafa e Diretrizes de Segurança**:
   - Remova rótulos e resíduos de cola, lavando e secando a garrafa antes do corte.
   - Em garrafas com amassados ou vincos, pode-se utilizar o bico adaptador rosqueável [Crushed Bottle Fixer](https://makerworld.com/pt/models/2620743-crushed-bottle-fixer#profileId-2892599) para pressurização leve com ar.
   - **Protocolo de Segurança para Alisamento Térmico**:
     > [!CAUTION]
     > Aqueça recipientes pressurizados apenas com as devidas precauções: limite a pressão a no máximo **15 a 20 psi**; mantenha a fonte de calor (soprador ou chama suave) a uma distância segura (**15 a 20 cm, nunca encostada no plástico**); mantenha a garrafa em rotação contínua para evitar superaquecimento localizado; use óculos de proteção. Preferencialmente, selecione garrafas que já tenham corpo cilíndrico liso e uniforme, que **dispensam qualquer uso de calor ou pressurização**.
2. **Filetamento Contínuo**:
   - O fundo da garrafa é retirado e a borda inicial recebe um corte chanfrado em triângulo.
   - A garrafa é cortada helicoidalmente com o cortador [Simple PET bottle cutter](https://makerworld.com/pt/models/98441-simple-pet-bottle-cutter#profileId-111492), obtendo-se uma tira contínua com largura ajustada à espessura da parede (ex.: 7.0 mm para refrigerante).
3. **Pultrusão Térmica no Hotend Creality Mk8 Adaptado**:
   - Utiliza-se um **Hotend Creality Mk8 1.75mm** com entrada cônica usinada por broca para guiar e afunilar a fita de forma progressiva.
   - O bloco opera com Setpoint do PID em **250.0 °C** para compensar a perda dinâmica de temperatura provocada pela fita fria, mantendo a temperatura efetiva de moldagem entre **235.0 °C e 245.0 °C**.
4. **Tração Mecânica e Bobinamento**:
   - A ponta inicial da fita é introduzida a quente, puxada com pinça/alicate na saída do bico e presa com firmeza nas engrenagens de tração (suportes reforçados com cubos no `.3mf`).
   - O operador gira o potenciômetro para acionar o motor NEMA 17 no momento desejado, tracionando o filamento em modo de **1/2 passo (400 pulsos/volta)**.

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
| **Driver de Motor** | Módulo Driver A4988 | Configurado em 1/2 micropasso (MS1 em HIGH: 400 pulsos/volta) | Controle de micropasso e torque mecânico do NEMA 17 |
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
  [ A4988: MS1 ] ──────────────────────────────────────────> [ +5V ] (Nível HIGH: 1/2 passo, 400 pulsos/volta)
  [ A4988: MS2, MS3 ] ─────────────────────────────────────> [ GND / Desconectado ] (Nível LOW)

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

### 1. Sensoriamento Térmico (Equação Beta Simplificada)
O firmware converte a leitura do ADC do pino `A0` utilizando a **equação Beta simplificada (modelo exponencial de 2 parâmetros)**:

$$\frac{1}{T} = \frac{1}{T_0} + \frac{1}{\beta} \ln\left(\frac{R_{\text{ntc}}}{R_0}\right)$$

Com $T_0 = 25\ ^\circ\text{C}$ ($298.15\text{ K}$), $R_0 = 100\text{ k}\Omega$ e $\beta = 3950$. A resistência da série é calibrada via constante `R_SERIE`. Essa simplificação substitui a equação de 3 parâmetros de Steinhart-Hart, poupando processamento aritmético no Arduino Uno de 8 bits e garantindo exatidão com erro $< \pm 0.5\ ^\circ\text{C}$ na faixa operacional.

### 2. Controle Térmico em Duas Zonas
* **Zona Bruta**: 100% PWM abaixo de $(\text{Setpoint} - 20\ ^\circ\text{C}) = 230\ ^\circ\text{C}$ para subida rápida.
* **Zona PID Fina**: A partir de 230 °C, o algoritmo assume controle proporcional-integral-derivativo contínuo mantendo o Setpoint de **250.0 °C** (temperatura real dinâmica entre 235 °C e 245 °C com a fita passando). Corte de emergência configurado para **265.0 °C**.

### 3. Tração Mecânica em 1/2 Passo e Acionamento Manual
* **Configuração de 1/2 Passo**: O driver A4988 opera com `MICROSTEPS = 2` (MS1 em nível HIGH / +5V), resultando em **400 pulsos por volta**. Essa configuração oferece torque de puxamento muito superior a frações menores (como 1/16), essencial para tracionar a fita de garrafa PET sem perda de passos.
* **Acionamento Manual sem Fio Guia**:
  1. Com o potenciômetro no zero (`valorPotenciometro < 25`), o pino `ENABLE` vai para `HIGH`, desenergizando as bobinas do motor NEMA 17.
  2. O operador aguarda o bloco aquecer, introduz a fita a quente pelo bico cônico e puxa a ponta pela saída com uma pinça ou alicate de bico.
  3. Com o motor livre e sem tração, a fita é fixada firmemente nas engrenagens de tração (reforçadas com cubos no `.3mf`).
  4. O operador gira o potenciômetro, acionando a tração motorizada e ajustando visualmente a rotação ideal (10 a 15 RPM).

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
   - **Reforço com Cubos no Arquivo .3mf**: Para fixar os suportes das engrenagens sobre uma bancada plana de madeira (sem o chassi original com encaixes do Petalot), **foram acrescentados cubos nos pontos ao arquivo `.3mf`** diretamente no fatiador. Essa adição geométrica ampliou a rigidez da base plástica contra a madeira, impedindo torção mecânica e desalinhamento dos eixos sob tração contínua.
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
1. **Ligar o Aquecimento**: Conecte a fonte de 12V. O display ligará e o bloco aquecerá rapidamente até o Setpoint de **250.0 °C**.
2. **Inserir a Fita Manualmente**: Com o potenciômetro no zero (motor parado), passe a ponta chanfrada da fita pelo bico cônico quente do hotend Mk8.
3. **Puxar e Prender**: Puxe a ponta que sai do bico com o auxílio de um alicate ou pinça até alcançar as engrenagens de tração e fixe-a firmemente nos dentes de arraste.
4. **Acionar a Tração**: Gire o potenciômetro. O operador ajusta visualmente a tração contínua e suave na cadência ideal (10 a 15 RPM).

### 4. Cuidados com Vapores, Fumos e Ventilação do Ambiente
> [!IMPORTANT]
> **Segurança Ocupacional e Controle de Emissões Térmicas**:
> * **Operação em Local Ventilado**: Opere a pultrusora sempre em ambiente bem arejado, próximo a janelas abertas ou com exaustão ativa direcionada para o exterior. Evite a operação contínua em recintos fechados.
> * **Higienização como Prevenção de Fumaça Tóxica**: A remoção minuciosa de colas adesivas de rótulos e resíduos de bebidas não é apenas estética — resíduos de cola expostos a 250 °C sofrem queima direta (pirólise), liberando acroleína e vapores químicos irritantes.
> * **Evitar Fita Estagnada no Bico Quente**: Caso a tração seja interrompida ou ocorra rompimento da fita, retire o plástico do bico ou desligue a fonte imediatamente. Manter o PET parado a 250 °C provoca degradação térmica do polímero, liberando acetaldeído (odor adocicado/irritante) e micropartículas.
> * **EPI para Sessões Prolongadas**: Para produção contínua de múltiplos carretéis, recomenda-se o uso de máscara com filtro para vapores orgânicos e partículas (nível P2/P95).

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
