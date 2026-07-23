# Pinout — D1 Mini (ESP8266)

```
D1 Mini GPIO        Componente
─────────────────────────────────────────
D0  (GPIO16)  ——  Faros frontales (PWM sw)
D1  (GPIO5)   ——  Intermitentes IZQ (front+rear)
D2  (GPIO4)   ——  Intermitentes DER (front+rear)
D3  (GPIO0)   ——  Freno (PWM sw)
D4  (GPIO2)   ——  Altavoz (PWM sw)
D5  (GPIO14)  ——  PWM Motor IZQ (hw PWM)
D6  (GPIO12)  ——  PWM Motor DER (hw PWM)
D7  (GPIO13)  ——  L298N IN1 (dir motor IZQ A)
D8  (GPIO15)  ——  L298N IN2 (dir motor IZQ B)
RX  (GPIO3)   ——  L298N IN3 (dir motor DER A)
TX  (GPIO1)   ——  L298N IN4 (dir motor DER B)
A0  (ADC)     ——  Bateria (divisor voltaje)

3.3V  ——  ESP8266 VCC
5V    ——  L298N VCC, LEDs via resistor
GND   ——  GND comun
```

## L298N Wiring

```
L298N Pin        D1 Mini
──────────────────────────
ENA (PWM A)  ——  D5 (GPIO14)
IN1          ——  D7 (GPIO13)
IN2          ——  D8 (GPIO15)
IN3          ——  RX (GPIO3)
IN4          ——  TX (GPIO1)
ENB (PWM B)  ——  D6 (GPIO12)
12V/VCC      ——  Powerbank 5V (via regulador)
GND          ——  GND comun
OUT1/OUT2    ——  Motor IZQ
OUT3/OUT4    ——  Motor DER
```

## LEDs

```
LED                  GPIO   Resistor
─────────────────────────────────────
Faros frontales x2   D0     220Ω cada uno (paralelo)
Intermitente IZQ     D1     220Ω
Intermitente DER     D2     220Ω
Freno                D3     220Ω

Cada intermitente controla 2 LEDs (front+rear) en paralelo.
```

## Altavoz

```
GPIO    Componente
──────────────────
D4      Altavoz piezoeléctrico o speaker 8Ω + transistor 2N2222
```
