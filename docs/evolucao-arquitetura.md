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

## Tração

- O motor de passo NEMA 17 é acionado pelo driver A4988, com STEP em D4, DIR em D5 e ENABLE em D6.
- O potenciômetro ligado a A1 mantém o motor desabilitado quando a leitura é inferior a 25. A partir desse valor, o firmware calcula uma rotação entre 1 e 30 RPM e habilita o driver.
- O cálculo de velocidade no código usa 200 passos completos por volta e `MICROSTEPS = 2`, isto é, 400 pulsos por volta. Os comentários nessa seção também citam 3.200 pulsos por volta e MS1, MS2 e MS3 conectados a +5 V, informações que não correspondem à expressão calculada. O firmware não controla esses pinos, então a rotação real depende da configuração física do A4988 e não é confirmada pela rotação calculada.
- A geração dos passos usa `micros()`; o pulso inclui uma espera de 3 microssegundos com `delayMicroseconds()`. Assim, a geração de passos não é totalmente livre de espera bloqueante.
- O enable do motor depende do potenciômetro, não da temperatura do bloco. O firmware não implementa o intertravamento térmico de motor descrito em versões anteriores da documentação.

## Outros componentes descritos no projeto

O README descreve um Arduino Uno, display TM1637, módulo MOSFET para o cartucho aquecedor, hotend adaptado e fonte de 12 V. As referências de modelo e ligações são informações de montagem; não constituem, por si só, evidência de desempenho ou de segurança elétrica da montagem.

## Escopo dos ensaios documentados

Os registros experimentais disponíveis neste repositório se limitam a configurações do fatiador. Não foram mantidos resultados ou medições que sustentem as afirmações anteriores sobre resistência mecânica, comportamento térmico do filamento, compatibilidade com impressoras, qualidade de impressão, largura ideal de corte ou desempenho mecânico da tração. Consulte [`ensaios-e-testes-praticos.md`](./ensaios-e-testes-praticos.md) para as configurações de fatiador registradas e seus limites.
