# Ensaios Práticos, Geometria do Filamento e Adaptações Mecânicas

Este documento detalha as conclusões empíricas de bancada obtidas nos testes de conformação por pultrusão térmica, caracterização física do filamento reciclado, calibração de fatiamento para impressão 3D (com destaque para a **Bambu Lab A1**) e adaptações mecânicas estruturais.

---

## 1. A Natureza Oca do Filamento e a Calibração de Fluxo (OrcaSlicer / Bambu Studio)

### A Seção Tubular Oca
Diferente dos filamentos industriais fabricados por extrusão por rosca fundida contínua (que geram um cilindro sólido maciço), a pultrusão de garrafas PET realiza a conformação geométrica de uma tira plana de espessura de parede típica entre $0.15\text{ mm}$ e $0.40\text{ mm}$.

Ao transpassar o bico cônico do **Hotend Creality Mk8**, a fita curva-se e solda longitudinalmente suas bordas, formando um **perfil anular tubular fechado (oco no interior)** com diâmetro externo calibrado em $1.75\text{ mm}$.

```text
       Filamento Comercial Sólido                Filamento PET por Pultrusão
         ┌─────────────────┐                         ┌─────────────────┐
       ┌─┘                 └─┐                     ┌─┘                 └─┐
      │█████████████████████│                     │████             ████│
      │█████ [ MACIÇO ] █████│                     │██   [ NÚCLEO ]   ██│
      │█████████████████████│                     │██    [  OCO  ]   ██│
       └─┐                 ┌─┘                     └─┐                 ┌─┘
         └─────────────────┘                         └─────────────────┘
      Massa linear: ~2.98 g/m                     Massa linear: ~1.20 - 1.80 g/m
```

### Compensação no Fatiador (Calibração de Fluxo PET)
Como o filamento de 1.75 mm exterior possui uma cavidade oca, o volume de plástico efetivo introduzido na impressora por milímetro tracionado é menor do que o calculado pelo fatiador padrão.

Para garantir que a peça impressa atinja densidade total, sem subextrusão ou vazios internos entre camadas (*air gaps*), foram validados os seguintes parâmetros empíricos (testados com o modelo de calibração *CaliCat*):

1. **Multiplicador de Fluxo (Flow Ratio / Extrusion Multiplier)**:
   - Configurado entre **$1.15$ e $1.35$ ($115\%$ a $135\%$)**. O valor de **$1.35$** (aumento de 35% na taxa de extrusão) forneceu fechamento perfeito de topo (*top surface*), sem linhas aparentes e com total adesão entre perímetros.
2. **Ajuste Alternativo via Diâmetro Equivalente**:
   - Outra abordagem validada com sucesso consiste em cadastrar o filamento no fatiador com diâmetro nominal de **$1.30\text{ mm}$ a $1.35\text{ mm}$** (mantendo o fluxo em 1.0). Isso instrui matematicamente o algoritmo a compensar a área reduzida de plástico.
3. **Perfil Térmico de Impressão**:
   - **Nozzle**: Primeira camada a **250 °C** e camadas subsequentes a **255 °C**. Essa temperatura assegura viscosidade fluida e excelente interdifusão molecular entre as camadas de PET.
   - **Mesa (Bed)**: **70 °C a 80 °C** em placa PEI texturizada, eliminando problemas de descolamento (*warping*).
4. **Distância de Retração**:
   - Parametrizada em **$0.4\text{ mm}$** para sistemas *Direct Drive*. Retrações longas devem ser rigorosamente evitadas, pois o filamento oco pode deformar na zona de transição fria do hotend.
5. **Velocidade Volumétrica Máxima**:
   - Limitada a **$6.0\text{ mm}^3/\text{s}$**, prevenindo sobrecarga na engrenagem de tração da impressora e assegurando tempo suficiente de fusão do PET.

---

## 2. Geometria Transversal: Formato em "O" vs. Perfil em "C"

```text
       Perfil Aberto ("C")                     Perfil Fechado ("O")
     [ Incompatível / Falha ]                  [ Nominal / Perfeito ]
         ┌─────────┐                                 ┌─────────┐
       ┌─┘         └─┐                             ┌─┘         └─┐
      │               │                           │       █       │
      │   (Abertura)  │                           │   (Soldado)   │
       └─┐         ┌─┘                             └─┐         ┌─┘
         └─────────┘                                 └─────────┘
   - Colapso sob compressão                  - Seção circular rígida
   - Patinamento nas engrenagens             - Tração suave de 1.75 mm
   - Entupimento no hotend                   - Compatível com Bambu Lab A1
```

### Impacto Crítico na Extrusora da Bambu Lab A1
A impressora **Bambu Lab A1** utiliza um sistema extrusor *Direct Drive* de engrenagens duplas em aço endurecido com elevada pressão de contato pontual.
* **Comportamento com Perfil em "C"**: Se a fita não fundir suas bordas e mantiver uma fenda longitudinal aberta (perfil em "C"), a pressão das engrenagens da Bambu Lab A1 **amassa e achata o filamento**. O plástico entra em colapso mecânico, perde tração, patina nas ranhuras e interrompe o avanço.
* **Requisito Obrigatório**: O processo de pultrusão deve impreterivelmente gerar um perfil fechado em **"O" completo**, garantindo integridade tubular e resistência à compressão radial.

---

## 3. Relação entre Espessura da Garrafa e Largura do Corte

Para garantir que a tira dobre no diâmetro exato de 1.75 mm sem faltar nem sobrar massa, a largura de corte deve ser ajustada inversamente à espessura da parede do PET:

| Tipo de Garrafa | Espessura da Parede ($e$) | Largura do Corte | Comportamento na Conformação | Desempenho do Motor NEMA 17 |
| :--- | :---: | :---: | :--- | :--- |
| **Refrigerante (Coca-Cola / Guaraná)** | $\approx 0.30\text{ mm}$ | **$7.0\text{ mm}$** | **Perfeito**. Fecha em formato "O" completo e tubular. | Tração estável, motor frio e sem perda de passos. |
| **Água Mineral Lisa (Fina)** | $\approx 0.15\text{ mm} - 0.20\text{ mm}$ | **$7.0\text{ mm}$** | **Insuficiente**. Falta massa; forma perfil em "C" que falha na Bambu Lab A1. | Sem esforço mecânico, mas filamento inutilizável. |
| **Água Mineral Lisa (Fina)** | $\approx 0.15\text{ mm} - 0.20\text{ mm}$ | **$10.0\text{ mm} - 12.0\text{ mm}$** | **Adequado**. Compensa a espessura fina e fecha o tubo circular. | Tração suave e diâmetro regular. |
| **Água com Gás (Espessa)** | $\approx 0.35\text{ mm} - 0.45\text{ mm}$ | **$7.0\text{ mm}$** | **Excessivo**. Fita espessa acumula massa e emperra no bico. | **Falha crítica**: Motor **perde passos** por sobretorque. |
| **Água com Gás (Espessa)** | $\approx 0.35\text{ mm} - 0.45\text{ mm}$ | **$5.0\text{ mm} - 5.5\text{ mm}$** | **Adequado**. Alivia o atrito e permite conformação fluida. | Tração restabelecida dentro dos parâmetros nominais. |

---

## 4. Ensaios de Resistência Mecânica e Térmica (Dados Petalot / rPET)

Testes de tração e caracterização mecânica compilados em projetos de referência da comunidade ([Petalot](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357)) e relatórios laboratoriais revelam propriedades notáveis no filamento reciclado:
* **Resistência à Tração**: Corpos de prova padronizados impressos em 100% de garrafas PET atingem resistência entre **40 MPa e 60 MPa**, valor comparável ao PETG virgem e muito superior a PLAs de baixo custo.
* **Módulo de Young (Rigidez)**: O rPET de garrafas possui alta orientação molecular herdada do processo de sopro industrial birrotacional das embalagens, conferindo rigidez estrutural superior à do PETG comum.
* **Resistência à Temperatura**: Enquanto o PLA deforma a partir de 55 °C, o filamento rPET tolera exposição contínua a **70 °C–80 °C** sem perda de forma, sendo indicado para peças automotivas e gabinetes de eletrônicos.

---

## 5. Adaptações Mecânicas e Modelos 3D Utilizados

### 1. Preparação da Garrafa: Alisamento Térmico com Ar Comprimido
* **Bico Rosqueável**: Utilizou-se o modelo [Crushed Bottle Fixer (MakerWorld)](https://makerworld.com/pt/models/2620743-crushed-bottle-fixer#profileId-2892599).
* **Técnica de Remoção de Vincos**: Após pressurizar a garrafa com ar, utiliza-se um soprador térmico ou a chama da boca do fogão posicionada **próxima, mas nunca encostada**, mantendo a garrafa girando. O calor suave encolhe as tensões superficiais enquanto o ar expande o plástico, eliminando 100% dos relevos antes do fatiamento.

### 2. Gabarito Cortador de Fita
* Para cortar a tira helicoidal contínua, utilizou-se o modelo [Simple PET bottle cutter (MakerWorld)](https://makerworld.com/pt/models/98441-simple-pet-bottle-cutter#profileId-111492), com lâmina de estilete padronizada de 18 mm.

### 3. Engrenagens de Tração do Projeto Petalot e Reforço Estrutural
* Foram adotadas as engrenagens de tração e mancais do projeto [Petalot (MakerWorld)](https://makerworld.com/pt/models/190886-petalot-plastic-bottles-into-3d-filament#profileId-2108357).
* **Adaptação Estrutural para Base em Madeira**:
  - Na montagem direta sobre bancada plana de madeira, a força axial de tração gerava torção nos suportes plásticos, fazendo as engrenagens perderem o paralelismo.
  - **Solução**: Foram introduzidas **duas barras fixas metálicas transversais** interligando diretamente as laterais dos suportes. Isso conferiu rigidez absoluta ao conjunto mecânico, eliminando folgas e perda de passos.
