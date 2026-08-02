AUTITO RC - Wemos D1 Mini ESP8266 + L298N
==========================================

RED WIFI
- Nombre: autito-rc
- Contraseña: 12345678
- Dirección: http://192.168.4.1

CARGA EN ARDUINO IDE
1. Instala el paquete de placas ESP8266.
2. Selecciona: LOLIN(WEMOS) D1 R2 & mini.
3. DESCONECTA temporalmente los cables L298N IN3 e IN4 de RX/TX.
   Esos pines son usados por el puerto USB durante la carga.
4. Abre autito_rc_wemos.ino y súbelo.
5. Desconecta el USB o apaga el circuito y vuelve a conectar IN3 e IN4.
6. Enciende, conéctate a autito-rc y abre http://192.168.4.1 en Chrome.

CONEXIÓN
- D5/GPIO14 -> ENA
- D6/GPIO12 -> ENB
- D7/GPIO13 -> IN1
- D8/GPIO15 -> IN2
- RX/GPIO3  -> IN3
- TX/GPIO1  -> IN4
- GND Wemos -> GND L298N

IMPORTANTE
- Retira los jumpers ENA y ENB del L298N porque esos pines reciben PWM.
- No alimentes los motores desde el Wemos.
- Usa tierra común entre Wemos, L298N y fuente de motores.
- Recomendado: resistencia de 10 kΩ desde ENA a GND y otra desde ENB a GND.
  Esto mantiene los motores deshabilitados durante el arranque y la carga.
- TX emite mensajes breves al arrancar; el pulldown en ENB evita movimientos.
- D8/GPIO15 debe permanecer en LOW durante el arranque.
- No uses Serial/Monitor serial: RX y TX controlan el motor derecho.

FUNCIONES
- Página web integrada, sin archivos externos.
- Punto de acceso y portal cautivo.
- Inclinación proporcional con calibración.
- Joystick proporcional alternativo.
- Giro tipo tanque.
- Sin movimientos diagonales: se usa el eje dominante.
- Rampa rápida de aceleración (~60 ms).
- Parada inmediata al soltar el joystick o volver al centro.
- Parada automática tras 450 ms sin órdenes.

NOTA SOBRE INCLINACIÓN
Los navegadores modernos pueden bloquear DeviceOrientation en páginas HTTP.
El programa detecta el bloqueo y cambia al joystick. Si ocurre incluso abriendo
192.168.4.1 en Chrome, para inclinación garantizada será necesaria una app móvil
o una solución HTTPS con certificado confiable.
