#include <Arduino.h>
#include <PID_v1.h>
#include <TM1637Display.h>
#include <math.h>

// ==========================================
// CONFIGURAÇÃO DOS PINOS
// ==========================================
// Sensor Térmico e Aquecedor
#define SENSOR_PIN A0
const int pinoPWM = 9;

// Pinos do Display TM1637 (4 dígitos)
#define CLK_PIN 2
#define DIO_PIN 3

// Pinos do Driver Motor de Passo A4988 (Mecanismo Puxador de Filamento NEMA 17)
#define PINO_STEP 4
#define PINO_DIR 5
#define PINO_ENABLE 6

// Pino Analógico do Potenciômetro para Ajuste de Velocidade de Tração
#define PINO_POT_VELOCIDADE A1

// ==========================================
// INSTÂNCIAS E SEGMENTOS DO DISPLAY
// ==========================================
TM1637Display display(CLK_PIN, DIO_PIN);

// Segmentos para a palavra "Err " no display TM1637 em caso de falha do sensor
const uint8_t SEG_ERR[] = {
  SEG_A | SEG_F | SEG_E | SEG_D | SEG_G, // E
  SEG_E | SEG_G,                         // r
  SEG_E | SEG_G                          // r
};

// ==========================================
// PARÂMETROS DO NTC 100K
// ==========================================
const float R_SERIE = 10000.0;    // Resistor de precisão do divisor (Nominal 10k)
const float R_NOMINAL = 100000.0; // Sensor NTC 100k
const float T_NOMINAL = 25.0;
const float BETA = 3950.0;        // Fator Beta NTC gotinha 100k

// Limites de proteção ADC
const int LEITURA_MIN_ADC = 10;   // Detecta curto-circuito
const int LEITURA_MAX_ADC = 1000; // Detecta circuito aberto

// ==========================================
// VARIÁVEIS E GANHOS DO PID
// ==========================================
double Setpoint, Input, Output;

// Ganhos calibrados para Bloco Aquecedor Hotend (Padrão Marlin 3D)
double Kp = 22.2, Ki = 1.08, Kd = 114.0;

// PID em modo DIRECT para controle de aquecimento
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// ==========================================
// PARÂMETROS DO MOTOR NEMA 17 E DRIVER A4988
// ==========================================
const int PASSOS_POR_REVOLUCAO = 200; // Passo angular de 1.8° por passo padrão NEMA 17
const int MICROSTEPS = 2;            // Configuração 1/2 passo (MS1 em HIGH/+5V, MS2 e MS3 em LOW/GND)
const long PASSOS_TOTAIS_VOLTA = (long)PASSOS_POR_REVOLUCAO * MICROSTEPS; // 400 pulsos/volta

// Faixas de operação de velocidade (RPM no eixo do motor / carretel)
// Estudos empíricos (REPET / IFS Lagarto) indicam operação ideal entre 10 e 15 RPM (25 a 52 cm/min)
const float VELOCIDADE_MAX_RPM = 30.0;
const float VELOCIDADE_MIN_RPM = 1.0;

// Variáveis de controle de passo e velocidade
bool motorHabilitado = false;
float rpmAtual = 0.0;
int valorPotenciometro = 0; // Armazena a leitura analógica do ADC (0 a 1023)
unsigned long intervaloPassoMicros = 0;
unsigned long ultimoPassoMicros = 0;

// ==========================================
// TEMPORIZAÇÃO NÃO-BLOQUEANTE
// ==========================================
unsigned long ultimaAtualizacaoTermica = 0;
const unsigned long INTERVALO_TERMICO_MS = 100; // Amostragem do NTC e cálculo PID a cada 100ms

// ==========================================
// PROTÓTIPOS DE FUNÇÕES
// ==========================================
float lerTemperatura();
void pidSetUp();
void atualizarVelocidadeMotor();
void desativarMotor();
inline void executarPassoMotor();

// ==========================================
// SETUP
// ==========================================
void setup()
{
  Serial.begin(9600);
  
  // 1. Inicialização do pino PWM do aquecedor (MOSFET)
  pinMode(pinoPWM, OUTPUT);
  analogWrite(pinoPWM, 0); // Inicia 100% desligado por segurança

  // 2. Inicialização dos pinos do Driver A4988
  pinMode(PINO_STEP, OUTPUT);
  pinMode(PINO_DIR, OUTPUT);
  pinMode(PINO_ENABLE, OUTPUT);

  // Inicia desligado: no A4988 nível HIGH no ENABLE desenergiza as bobinas
  digitalWrite(PINO_ENABLE, HIGH);
  digitalWrite(PINO_DIR, LOW); // Sentido anti-horário (tração do filamento)
  digitalWrite(PINO_STEP, LOW);

  // 3. Inicialização do Display TM1637
  display.setBrightness(0x0f);
  display.clear();

  // 4. Configuração inicial do PID
  pidSetUp();

  // 5. Inicialização da tração (o motor gira desde o início se o potenciômetro estiver aberto)
  atualizarVelocidadeMotor();
}

// ==========================================
// LOOP PRINCIPAL (100% NÃO-BLOQUEANTE)
// ==========================================
void loop()
{
  unsigned long agoraMillis = millis();

  // -------------------------------------------------------------
  // CICLO 1: CONTROLE TÉRMICO E TELEMETRIA (A CADA 100 MS)
  // -------------------------------------------------------------
  if (agoraMillis - ultimaAtualizacaoTermica >= INTERVALO_TERMICO_MS)
  {
    ultimaAtualizacaoTermica = agoraMillis;

    // Atualiza a velocidade e acionamento do motor continuamente
    atualizarVelocidadeMotor();

    float temperatura = lerTemperatura();

    // --- 1. FALHA CRÍTICA DO SENSOR (Curto ou Circuito Aberto) ---
    if (isnan(temperatura))
    {
      analogWrite(pinoPWM, 0); // Desliga o aquecedor imediatamente por segurança
      myPID.SetMode(MANUAL);   // Desativa o PID

      // Exibe "Err" no display TM1637
      display.clear();
      display.setSegments(SEG_ERR, 3, 0);

      Serial.print(F("!!! ERRO CRÍTICO: SENSOR FALHOU | Pot: "));
      Serial.print(valorPotenciometro);
      Serial.print(F(" | Motor: "));
      if (motorHabilitado)
      {
        Serial.print(F("LIGADO ("));
        Serial.print(rpmAtual, 1);
        Serial.println(F(" RPM)"));
      }
      else
      {
        Serial.println(F("PARADO (POT EM ZERO)"));
      }
    }
    // --- 2. PROTEÇÃO DE SOBRETEMPERATURA (> 265°C) ---
    else if (temperatura > 265.0)
    {
      analogWrite(pinoPWM, 0); // Desliga o aquecedor imediatamente
      myPID.SetMode(MANUAL);   // Desativa o PID

      int tempExibir = (int)round(temperatura * 10.0);
      display.showNumberDecEx(tempExibir, 0b00100000, false, 4, 0);

      Serial.print(F("!!! ALERTA: SOBRETEMPERATURA (>265°C) | Temp: "));
      Serial.print(temperatura, 1);
      Serial.print(F("°C | Pot: "));
      Serial.print(valorPotenciometro);
      Serial.print(F(" | Motor: "));
      if (motorHabilitado)
      {
        Serial.print(F("LIGADO ("));
        Serial.print(rpmAtual, 1);
        Serial.println(F(" RPM)"));
      }
      else
      {
        Serial.println(F("PARADO (POT EM ZERO)"));
      }
    }
    // --- 3. OPERAÇÃO NORMAL ---
    else
    {
      Input = temperatura;

      // Atualiza display com temperatura atual (3 dígitos + 1 decimal)
      int tempExibir = (int)round(temperatura * 10.0);
      display.showNumberDecEx(tempExibir, 0b00100000, false, 4, 0);

      // Estratégia em Duas Zonas: Aquecimento bruto (< Setpoint - 20°C) ou PID Fino
      if (temperatura < (Setpoint - 20.0))
      {
        myPID.SetMode(MANUAL);
        Output = 255;
        analogWrite(pinoPWM, 255);
      }
      else
      {
        if (myPID.GetMode() == MANUAL)
        {
          myPID.SetMode(AUTOMATIC);
        }
        myPID.Compute();
        analogWrite(pinoPWM, Output);
      }

      // Telemetria via Serial Monitor com leitura do Potenciômetro
      Serial.print(F("Atual: "));
      Serial.print(temperatura, 1);
      Serial.print(F("°C | Alvo: "));
      Serial.print(Setpoint, 1);
      Serial.print(F("°C | PWM: "));
      Serial.print(Output, 0);
      Serial.print(F(" | Pot: "));
      Serial.print(valorPotenciometro);
      Serial.print(F(" | Motor: "));
      if (motorHabilitado)
      {
        Serial.print(F("LIGADO ("));
        Serial.print(rpmAtual, 1);
        Serial.println(F(" RPM)"));
      }
      else
      {
        Serial.println(F("PARADO (POT EM ZERO)"));
      }
    }
  }

  // -------------------------------------------------------------
  // CICLO 2: GERAÇÃO CONTÍNUA E NÃO-BLOQUEANTE DE PASSOS (A4988)
  // -------------------------------------------------------------
  executarPassoMotor();
}

// ==========================================
// FUNÇÕES AUXILIARES DE CONTROLE
// ==========================================

// Desliga e desenergiza o motor de passo (economiza energia e evita aquecimento)
void desativarMotor()
{
  if (motorHabilitado)
  {
    digitalWrite(PINO_ENABLE, HIGH); // Nível HIGH no A4988 desliga saídas das bobinas
    motorHabilitado = false;
    intervaloPassoMicros = 0;
    rpmAtual = 0.0;
  }
}

// Lê o potenciômetro e calcula a frequência de passos proporcional à rotação
void atualizarVelocidadeMotor()
{
  valorPotenciometro = analogRead(PINO_POT_VELOCIDADE);

  // Zona morta no início do potenciômetro para desligar o movimento (knob no mínimo)
  if (valorPotenciometro < 25)
  {
    desativarMotor();
    return;
  }

  // Mapeamento proporcional de RPM (1.0 a 30.0 RPM)
  rpmAtual = VELOCIDADE_MIN_RPM + ((float)(valorPotenciometro - 25) / (1023.0 - 25.0)) * (VELOCIDADE_MAX_RPM - VELOCIDADE_MIN_RPM);

  // Cálculo de passos por segundo e intervalo em microssegundos:
  // Passos/s = (RPM * PASSOS_TOTAIS_VOLTA) / 60
  float passosPorSegundo = (rpmAtual * (float)PASSOS_TOTAIS_VOLTA) / 60.0;
  if (passosPorSegundo > 0)
  {
    intervaloPassoMicros = (unsigned long)(1000000.0 / passosPorSegundo);
  }

  // Se o motor estava desligado, energiza o driver
  if (!motorHabilitado)
  {
    digitalWrite(PINO_ENABLE, LOW); // Nível LOW habilita o driver A4988
    motorHabilitado = true;
    ultimoPassoMicros = micros();
  }
}

// Gera pulso no pino STEP respeitando a temporização sem bloquear o microcontrolador
inline void executarPassoMotor()
{
  if (motorHabilitado && intervaloPassoMicros > 0)
  {
    unsigned long agoraMicros = micros();
    if (agoraMicros - ultimoPassoMicros >= intervaloPassoMicros)
    {
      ultimoPassoMicros = agoraMicros;
      digitalWrite(PINO_STEP, HIGH);
      delayMicroseconds(3); // Pulso mínimo exigido pelo A4988 (1 µs mínimo)
      digitalWrite(PINO_STEP, LOW);
    }
  }
}

// Amostragem do Termistor NTC 100k com conversão por Steinhart-Hart / Parâmetro Beta
float lerTemperatura()
{
  const float resolucao = 1023.0;
  long soma = 0;
  // 10 leituras rápidas e consecutivas no ADC para filtrar ruídos elétricos sem travar o processamento
  for (int i = 0; i < 10; i++)
  {
    soma += analogRead(SENSOR_PIN);
  }
  float leituraMedia = (float)soma / 10.0;

  if (leituraMedia < LEITURA_MIN_ADC || leituraMedia > LEITURA_MAX_ADC)
  {
    return NAN; // Sensor em curto ou circuito aberto
  }

  // Equação Steinhart-Hart através do Parâmetro Beta
  float resistencia = R_SERIE / ((resolucao / leituraMedia) - 1.0);
  float tempK = log(resistencia / R_NOMINAL) / BETA;
  tempK += 1.0 / (T_NOMINAL + 273.15);
  float tempC = (1.0 / tempK) - 273.15;

  return tempC;
}

// Configurações padrão do controlador PID
void pidSetUp()
{
  Setpoint = 250.0; // Ponto de trabalho estabelecido (compensa queda térmica com a passagem da fita mantendo 235-245°C)
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255);
}