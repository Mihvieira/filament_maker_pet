# Evolução da Arquitetura e Decisões de Engenharia

Este documento consolida o histórico das decisões de engenharia, ensaios práticos e iterações de projeto realizadas ao longo do desenvolvimento da máquina **Filament Maker**.

---

## 1. Do Termostato Comercial (W1209) ao Controle Embarcado Dedicado

### Contexto Inicial
A primeira concepção da máquina considerou o uso do módulo termostato digital **W1209** como interface autônoma de controle de temperatura do bloco aquecedor.

### Problemas e Limitações Identificadas
* **Faixa Térmica Inadequada**: O W1209 foi projetado com firmware e sensor padrão (NTC 10k) limitados a aproximadamente 110 °C, sendo inviável para atingir e sustentar os 250 °C exigidos na conformação do PET.
* **Histerese Oscilatória (Bang-Bang)**: O controle por relé mecânico provocava oscilações térmicas de ±15 °C no bloco aquecedor de alumínio, desestabilizando a viscosidade do polímero e gerando variações inaceitáveis de diâmetro.
* **Impossibilidade de Sincronismo com a Mecânica**: Não havia como integrar a leitura térmica com o chaveamento do motor de tração em uma única placa compacta.

### Decisão Técnica
* **Adoção do Arduino Uno R3**: Centralização de todo o sistema em um microcontrolador programável, permitindo amostragem rápida do sensor analógico, controle PID em alta frequência e geração precisa de pulsos de passo para o motor NEMA 17.

---

## 2. Sensoriamento Térmico: Termistor NTC 100k e Calibração Real

### Desafios de Leitura Analógica
A adoção do termistor de vidro tipo gotinha (NTC 100k 3950) revelou problemas comuns de bancada:
1. **Flutuações por Ruído Eletromagnético**: Ruídos induzidos pelas comutações de potência causavam picos espúrios na leitura analógica.
2. **Erros de Offset por Tolerância do Resistor**: A utilização de valores nominais teóricos para o resistor fixo do divisor de tensão causava erros de calibração de até 10 °C.

### Soluções Implementadas
* **Topologia do Divisor**: Resistor fixo conectado entre os `+5V` e o pino analógico `A0`, com o NTC 100k ligado entre `A0` e o `GND`.
* **Calibração Real em Bancada**: Medição da resistência real com multímetro de precisão e inserção direta na constante `R_SERIE` do firmware.
* **Equação de Steinhart-Hart / Coeficiente Beta**: Conversão com parâmetro $\beta = 3950$, assegurando linearidade matemática precisa até 265 °C.
* **Sobreamostragem e Média Móvel**: Média sobre 10 amostras rápidas sucessivas a cada ciclo de 100 ms, filtrando ruídos de alta frequência.

---

## 3. Sistema Térmico e de Potência: MOSFET HA210N06 e Hotend Creality Mk8

### Hotend Creality Mk8 com Entrada Cônica Adaptada
* Para o elemento aquecedor e conformador, adotou-se o conjunto **Hotend Creality Mk8 1.75mm** (utilizado nas impressoras Ender 3 Pro/V2 e CR10), equipado com cartucho cerâmico de 12V 40W.
* **Adaptação Mecânica do Bocal**: A entrada cilíndrica original foi **alargada e usinada com broca cônica**, criando uma transição em funil suave que força as bordas da fita de garrafa a se dobrarem continuamente em direção ao centro sem engavetamento.

### Chaveamento com Módulo MOSFET HA210N06
* Substituição de relés mecânicos pelo módulo MOSFET de potência **HA210N06**, amplamente utilizado em extrusoras de impressoras 3D comerciais devido à sua baixíssima resistência interna em condução ($R_{DS(on)}$).
* **Proteções de Gate**:
  - Resistor em série de **$220\,\Omega$** entre o pino `D9` do Arduino e o terminal Gate (G), limitando correntes de pico transitórias.
  - Resistor de **$10\text{ k}\Omega$ de pull-down** entre o Gate e o GND comum, garantindo que o aquecedor permaneça desligado caso o Arduino reinicie ou esteja em processo de inicialização.

### Dinâmica Térmica: Definição do Setpoint em 250 °C
* Embora a temperatura teórica ótima de moldagem do PET estático seja de cerca de 235 °C, definiu-se o **Setpoint do PID em 250.0 °C** no firmware.
* **Justificativa Dinâmica**: A fita contínua de PET entra fria (temperatura ambiente) no bloco a uma cadência contínua, retirando calor do sistema. Com o Setpoint em 250 °C, o equilíbrio dinâmico real do bico durante a operação fixa-se entre **235.0 °C e 245.0 °C**, garantindo alta velocidade de conformação, excelente fluidez e soldagem homogênea sem degradar o polímero.
* A trava de emergência por sobretemperatura foi ajustada para **265.0 °C**.

---

## 4. Mecanismo de Tração: Transição do Fio Guia para Acionamento Manual

### Falha da Estratégia de Fio Guia
* Inicialmente, testou-se o uso de um "fio guia" metálico ou plástico preso à ponta da fita para tentar transpassá-la a frio através do bico e iniciar o puxamento de forma automática.
* **Problema Encontrado**: O fio guia desprendia-se, entortava ou quebrava ao passar pela zona cônica aquecida, além de correr o risco de riscar as paredes internas do bocal usinado.

### Solução Definitiva: Acionamento Manual via Potenciômetro
* O firmware foi configurado para que o motor permaneça desligado enquanto o potenciômetro estiver em sua zona morta inicial (`valorPotenciometro < 25`).
* **Procedimento Operacional**:
  1. O bloco atinge a temperatura operacional de 250 °C.
  2. A ponta chanfrada em triângulo da fita é introduzida manualmente no bico cônico quente.
  3. Com um alicate de bico fino ou pinça, o operador puxa a ponta emergente pela saída do bico e a fixa firmemente nas engrenagens de tração (adaptadas do projeto [Petalot](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357)).
  4. O operador gira o knob do potenciômetro para ativar o motor NEMA 17 no instante exato, regulando suavemente a velocidade entre 10 e 15 RPM.

---

## 5. Resumo Comparativo das Decisões de Arquitetura

| Subsistema | Decisão Inicial / Descartada | Decisão Final Homologada | Motivação Técnica |
| :--- | :--- | :--- | :--- |
| **Controlador** | Termostato digital W1209 | Arduino Uno R3 com firmware C++ | Escala térmica até 265 °C, controle PID e flexibilidade de I/O. |
| **Sensor Térmico** | NTC 10k sem filtragem | NTC 100k 3950 + Resistor medido + Média móvel | Precisão em 250 °C, imunidade a ruídos e anulação de offset. |
| **Potência** | Relé eletromecânico | Módulo MOSFET HA210N06 (com $220\,\Omega$ e $10\text{ k}\Omega$) | Chaveamento PWM de alta velocidade, silencioso e seguro. |
| **Elemento Aquecedor**| Bloco genérico sem afunilamento | Hotend Creality Mk8 com entrada cônica brocada | Guia progressiva que dobra a fita de forma tubular contínua. |
| **Temperatura Setpoint**| 235 °C estático | 250.0 °C nominal (queda real para 235 °C–245 °C) | Compensa a perda de calor por condução com a passagem da fita. |
| **Inserção da Fita** | Fio guia de alimentação a frio | Inserção a quente com fixação e acionamento manual | Elimina falhas de desprendimento do guia no cone quente. |
| **Tração Mecânica** | Motor DC comum com redução genérica | Motor NEMA 17 + A4988 + Engrenagens com reforço | Passo angular firme, sem perda de torque e sem flexão na bancada. |
