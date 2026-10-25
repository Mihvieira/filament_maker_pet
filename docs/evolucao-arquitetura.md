# Evolução da Arquitetura e Decisões de Engenharia

Este documento consolida o histórico técnico das decisões de engenharia, ensaios práticos e iterações de projeto realizadas ao longo do desenvolvimento do **Filament Maker**.

---

## 1. Do Termostato Comercial (W1209) ao Controle Embarcado Dedicado

### Contexto Inicial
A primeira concepção da máquina avaliou o módulo termostato digital comercial **W1209** como controlador autônomo de temperatura do bloco aquecedor.

### Problemas e Limitações Identificadas
* **Escala Térmica Inadequada**: O W1209 de fábrica (com sensor NTC 10k) opera tipicamente até ~110 °C, sendo incapaz de atingir e sustentar os 250 °C requeridos na conformação do PET.
* **Histerese Bang-Bang com Relé**: O chaveamento por relé eletromecânico gerava oscilações térmicas de ±15 °C no bloco de alumínio, desestabilizando a viscosidade do plástico fundido e causando deformações no filamento.
* **Falta de Integração**: Impossibilidade de sincronizar o controle térmico com a lógica de tração mecânica.

### Decisão Técnica
* **Adoção do Arduino Uno R3**: Centralização de todo o sistema no microcontrolador ATmega328P, viabilizando leitura analógica precisa, modulação PWM de alta frequência via PID e temporização para geração de passos do motor.

---

## 2. Sensoriamento Térmico: Da Equação de Steinhart-Hart Completa à Equação Beta Simplificada

### Desafios de Leitura Analógica
A implementação do termistor tipo gotinha (NTC 100k 3950) demandou cuidados com estabilidade e modelagem matemática:
1. **Flutuações de Ruído Eletromagnético**: Ruídos induzidos pelo chaveamento PWM do aquecedor causavam variações no ADC. Solucionou-se com média móvel sobre 10 amostras a cada ciclo de 100 ms.
2. **Offset por Tolerância Comercial**: O uso do valor nominal do resistor de polarização causava erros sistemáticos. A resistência real é medida em bancada com multímetro e inserida na constante `R_SERIE` do código.

### Transição do Modelo Matemático: Steinhart-Hart vs. Equação Beta
* **Tentativa com a Equação Completa de Steinhart-Hart**:
  Inicialmente, considerou-se a equação polinomial clássica de Steinhart-Hart com três coeficientes:
  $$\frac{1}{T} = A + B \ln(R) + C (\ln R)^3$$
  * *Por que foi descartada?* A determinação dos coeficientes $A$, $B$ e $C$ exige levantamento experimental em três temperaturas de calibração com banho térmico de precisão laboratorial, o que criaria uma barreira de replicabilidade para a comunidade maker. Além disso, o cálculo contínuo de potências cúbicas em ponto flutuante consumia ciclos desnecessários de CPU no microcontrolador de 8 bits (16 MHz).
* **Decisão / Adoção da Equação Beta Simplificada (Modelo em Produção)**:
  Optou-se pela equação exponencial simplificada baseada no parâmetro Beta ($\beta$):
  $$\frac{1}{T} = \frac{1}{T_0} + \frac{1}{\beta} \ln\left(\frac{R_{\text{ntc}}}{R_0}\right)$$
  * *Parâmetros em Produção*: $T_0 = 298.15\text{ K}$ ($25\ ^\circ\text{C}$), $R_0 = 100\text{ k}\Omega$ e $\beta = 3950$.
  * *Justificativa*: Como o valor $\beta = 3950$ é fornecido e padronizado pelos fabricantes de termistores gotinha NTC 100k de impressoras 3D, a fórmula requer apenas uma operação de logaritmo natural (`log()`), atingindo exatidão com desvio inferior a ±0.5 °C na faixa de 200 °C a 265 °C, com execução leve e não-bloqueante no Arduino.

---

## 3. Sistema Térmico e de Potência: MOSFET HA210N06 e Hotend Creality Mk8

### Hotend Creality Mk8 com Entrada Cônica Adaptada
* Adotou-se o **Hotend Creality Mk8 1.75mm** (modelo da Ender 3 / CR10) com cartucho cerâmico de 12V 40W.
* **Adaptação por Broca Cônica**: A entrada cilíndrica foi usinada com broca cônica para criar um funil de entrada suave, forçando as abas da fita plástica a se dobrarem continuamente em direção ao centro sem emperrar.

### Módulo Driver PWM + MOSFET HA210N06
* Chaveamento de potência realizado pelo **Módulo Driver PWM MOSFET HA210N06** (15A / 400W), amplamente empregado em extrusoras comerciais e camas aquecidas por sua baixíssima resistência interna de condução ($R_{DS(on)}$).
* **Topologia e Conexões do Módulo**:
  - **Entrada de Controle**: Recebe o sinal de chaveamento do pino `D9` (PWM) do Arduino no terminal `TRIG / PWM`, com terra compartilhado no `GND` de sinal.
  - **Entradas de Alta Corrente (`VIN+` e `VIN-`)**: Conectadas diretamente aos barramentos de +12V e GND da fonte chaveada externa.
  - **Saídas de Potência (`OUT+` e `OUT-`)**: Conectadas diretamente aos terminais do cartucho cerâmico de 12V 40W do hotend Mk8.
  - O circuito interno do módulo já incorpora os circuitos de chaveamento rápido e pull-down necessários para manter o MOSFET desligado durante o boot do Arduino, eliminando ruídos e prevenindo acionamentos involuntários.

### Dinâmica Térmica: Definição do Setpoint em 250 °C
* Embora a moldagem estática do PET ocorra em ~235 °C, o firmware define o **Setpoint em 250.0 °C**.
* **Efeito Térmico Dinâmico**: A fita de garrafa entra fria continuamente no bloco em velocidade contínua, atuando como um dissipador de calor. O Setpoint em 250.0 °C compensa essa drenagem, mantendo a temperatura real de equilíbrio no bico entre **235.0 °C e 245.0 °C** durante a tração.
* A proteção de desligamento de emergência por sobretemperatura foi estabelecida em **265.0 °C**.

---

## 4. Mecanismo de Tração: 1/2 Passo e Acionamento Manual

### Configuração de Microstepping: 1/2 Passo no Driver A4988
No código em produção (`filament_maker.ino`):
```cpp
const int PASSOS_POR_REVOLUCAO = 200; // Passo angular de 1.8° por passo padrão NEMA 17
const int MICROSTEPS = 2;            // Configuração 1/2 passo (MS1 em HIGH, MS2 e MS3 em LOW)
const long PASSOS_TOTAIS_VOLTA = (long)PASSOS_POR_REVOLUCAO * MICROSTEPS; // 400 pulsos/volta
```
* **Configuração Física dos Pinos**:
  - `MS1`: Conectado a **+5V (HIGH)**.
  - `MS2` e `MS3`: Conectados ao **GND (LOW)** ou mantidos sem conexão (graças aos resistores internos de pull-down do módulo A4988).
* **Justificativa de Engenharia para o 1/2 Passo**:
  - **Torque Máximo sob Carga**: Ao contrário de frações menores como 1/16 (onde o torque incremental entre micropassos cai drasticamente), o modo de **1/2 passo** preserva cerca de 70.7% do torque nominal de retenção de pico do NEMA 17, essencial para vencer o atrito de pultrusão no bico cônico.
  - **Suavidade e Ruído**: Reduz a ressonância mecânica e vibração do passo cheio (Full Step) sem exigir frequências de pulsos excessivamente altas (em 10 a 15 RPM geram-se apenas ~66 a 100 pulsos por segundo), garantindo folga de processamento para a malha PID.

### Transição do Fio Guia para Acionamento Manual
* **Inviabilidade do Fio Guia**: A tentativa de guiar a fita com um fio condutor quebrava ou desprendia-se na zona de alta temperatura.
* **Metodologia Manual**: O motor permanece desenergizado com o potenciômetro no zero (`valorPotenciometro < 25`). O operador insere a ponta da fita a quente pelo bico, puxa-a pela saída com uma pinça ou alicate de bico e fixa-a nas engrenagens de tração. Em seguida, gira o potenciômetro para ligar o motor no instante adequado e dosar a rotação.

### Reforço dos Suportes de Engrenagem (Adição de Cubos no .3mf)
* Ao montar os suportes de engrenagem do projeto Petalot sobre uma bancada plana de madeira, identificou-se flexão sob a tensão do filamento.
* **Solução**: Em vez de usar perfis externos, foram **acrescentados cubos de reforço geométrico diretamente nos pontos de fixação do arquivo `.3mf`**, aumentando a base de contato e a rigidez do conjunto impresso.

---

## 5. Diretrizes Críticas de Segurança: Alisamento Térmico de Garrafas

> [!CAUTION]
> **Aviso de Segurança: Recipientes Pressurizados sob Ação de Calor**
> A aplicação de calor sobre corpos ocos inflados com ar comprimido envolve risco de rompimento por sobrepressão plástica localizada se não forem observados limites rígidos de operação.

### Protocolo de Redução de Riscos
1. **Limite Estrito de Pressão**: A pressão de inflagem no bico adaptador [Crushed Bottle Fixer](https://makerworld.com/pt/models/2620743-crushed-bottle-fixer#profileId-2892599) jamais deve ultrapassar **15 a 20 psi** (pressão leve, suficiente apenas para dar forma ao corpo plástico).
2. **Distância e Movimento Contínuo**: A fonte térmica (soprador térmico ou chama) deve ser posicionada a uma distância mínima de **15 a 20 cm, nunca encostando no plástico**. A garrafa deve ser mantida em rotação contínua (ex.: fixada em mandril de parafusadeira/furadeira). Jamais concentre o calor em uma área estática, pois isso provocará afinamento térmico localizado e estouro da parede.
3. **Uso Obrigatório de EPI**: Óculos de proteção com proteção lateral e luvas térmicas.
4. **Alternativa Segura Preferencial**: Dar preferência a garrafas que naturalmente já possuem corpo liso e cilíndrico (sem relevos profundos), as quais **dispensam qualquer uso de pressurização ou calor**, permitindo corte direto e seguro no gabarito cortador.

### Emissões Térmicas, Fumos e Ventilação do Ambiente
Durante a pultrusão a 250 °C, a integridade da atmosfera de trabalho depende da pureza do material processado:
* **Pirólise de Resíduos Adesivos**: Se restarem adesivos acrílicos ou colas de rótulo na garrafa, o contato direto com o bico aquecido a 250 °C causa combustão térmica incompleta (pirólise), liberando acroleína e compostos voláteis irritantes para a mucosa ocular e trato respiratório. A higienização química prévia (óleo/solvente) é, portanto, um requisito de biossegurança ocupacional.
* **Degradação do PET Estagnado**: Se a fita plástica permanecer parada dentro do bico aquecido sem tracionamento contínuo, o polímero degrada termo-oxidativamente, liberando acetaldeído (composto volátil com odor adocicado característico) e microaerossóis. O firmware previne isso ao permitir acionamento manual imediato ou corte rápido de energia.
* **Ventilação Obrigatória**: A operação da máquina deve ocorrer estritamente em locais arejados, próximo a aberturas ou sistemas de exaustão, sendo recomendável o uso de máscara P2/P95 em rotinas de fabricação contínua.

---

## 6. Resumo das Decisões de Arquitetura

| Subsistema | Decisão Inicial / Descartada | Decisão Final Homologada | Motivação Técnica |
| :--- | :--- | :--- | :--- |
| **Controlador** | Termostato digital W1209 | Arduino Uno R3 com firmware C++ | Escala térmica até 265 °C, controle PID e flexibilidade de I/O. |
| **Cálculo NTC** | Steinhart-Hart de 3 parâmetros | Equação Beta simplificada ($\beta=3950$) | Requer calibração simples de 1 ponto ($R_{\text{série}}$) e alivia a CPU do Arduino. |
| **Potência** | Relé eletromecânico | Módulo Driver PWM MOSFET HA210N06 | Chaveamento PWM silencioso de alta potência (15A/400W) com 2 entradas 12V e 2 saídas dedicadas para o hotend. |
| **Elemento Aquecedor**| Bloco genérico sem afunilamento | Hotend Creality Mk8 com entrada cônica | Transição gradual em funil que dobra a fita sem emperrar. |
| **Setpoint Térmico**| 235 °C estático | 250.0 °C nominal (queda real para 235 °C–245 °C) | Compensa a perda contínua de calor transferida para a fita fria. |
| **Microstepping** | 1/16 micropasso (3200 p/v) | 1/2 passo (400 pulsos/volta com MS1 no +5V) | Máximo torque útil de puxamento e frequência de pulsos moderada. |
| **Inserção da Fita**| Fio guia a frio | Inserção a quente com acionamento manual | Elimina falhas de desprendimento do fio guia no bocal quente. |
| **Estrutura Mecânica**| Peça plástica flexível sobre madeira | Suportes reforçados com cubos no `.3mf` | Elimina desalinhamento de engrenagens sem componentes extras. |
