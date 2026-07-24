/*
 * ============================================================================
 * ToritoRC — Firmware + WebServer + WebSocket Server para Arduino IDE
 * ============================================================================
 * 
 * Requisitos en Arduino IDE:
 * 1. Tarjeta: ESP8266 (NodeMCU 1.0 o Wemos D1 R2 & mini)
 * 2. Librerías (Gestor de Bibliotecas):
 *    - WebSockets (por Markus Sattler)
 *    - ArduinoJson (por Benoit Blanchon v6/v7)
 * 
 * ¡No requiere LittleFS ni comandos extra!
 * Con un solo clic en "Subir", se graba tanto el programa C++ como la Web completa.
 * 
 * Conexión:
 * - WiFi: SSID "ToritoRC", Pass "toritopass"
 * - Navegador: http://192.168.4.1
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// ============================================================================
// CONFIGURACIÓN DE PINES (Mismo mapeo de hardware)
// ============================================================================
#define PIN_FAROS          D0 // GPIO16 - Faros Delanteros
#define PIN_INT_IZQ        D1 // GPIO5  - Intermitente Izquierdo
#define PIN_INT_DER        D2 // GPIO4  - Intermitente Derecho
#define PIN_FRENO          D3 // GPIO0  - Luz de Freno
#define PIN_SPEAKER        D4 // GPIO2  - Altavoz / Speaker (Bocina PWM)

#define PIN_MOTOR_IZQ_PWM  D5 // GPIO14 - ENA (Velocidad Motor IZQ)
#define PIN_MOTOR_DER_PWM  D6 // GPIO12 - ENB (Velocidad Motor DER)
#define PIN_MOTOR_IZQ_IN1  D7 // GPIO13 - IN1 (Dirección A Motor IZQ)
#define PIN_MOTOR_IZQ_IN2  D8 // GPIO15 - IN2 (Dirección B Motor IZQ)
#define PIN_MOTOR_DER_IN3  RX // GPIO3  - IN3 (Dirección A Motor DER)
#define PIN_MOTOR_DER_IN4  TX // GPIO1  - IN4 (Dirección B Motor DER)

#define PIN_BATTERY        A0 // ADC    - Sensor Batería

// ============================================================================
// ESTADO Y VARIABLES GLOBALES
// ============================================================================
ESP8266WebServer server(80);
WebSocketsServer ws(81);

struct VehicleState {
  int leftMotor = 0;      // -255 a 255
  int rightMotor = 0;     // -255 a 255
  int gear = 0;           // 0=N, 1..5, 6=R
  bool frontLights = false;
  bool leftBlinker = false;
  bool rightBlinker = false;
  bool brake = false;
  bool horn = false;

  // Calculados
  int rpm = 800;
  float speedMps = 0.0f;
  float batteryVoltage = 7.4f;
} car;

unsigned long lastBlinkerToggle = 0;
bool blinkerState = false;
unsigned long lastTelemetrySend = 0;

// Multiplicador de potencia según la marcha seleccionada (0=Neutral, 1..5, 6=Reversa)
const float GEAR_RATIOS[7] = { 0.0f, 0.35f, 0.55f, 0.75f, 0.90f, 1.00f, 0.40f };

// ============================================================================
// PÁGINA WEB HTML5 + CSS3 + JAVASCRIPT (PROGMEM)
// ============================================================================
const char HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ToritoRC Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; user-select: none; -webkit-user-select: none; touch-action: manipulation; }
    body { background: #0c0a09; color: #f5f5f4; font-family: system-ui, -apple-system, sans-serif; height: 100vh; overflow: hidden; display: flex; flex-direction: column; }
    
    /* Header Status */
    header { background: #1c1917; padding: 10px 16px; display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid #292524; }
    .logo { font-weight: 800; font-size: 1.1rem; color: #f59e0b; display: flex; align-items: center; gap: 6px; }
    .badge { padding: 4px 10px; border-radius: 9999px; font-size: 0.75rem; font-weight: 600; display: flex; align-items: center; gap: 6px; }
    .connected { background: #064e3b; color: #34d399; border: 1px solid #059669; }
    .disconnected { background: #7f1d1d; color: #fca5a5; border: 1px solid #dc2626; }
    .dot { width: 8px; height: 8px; border-radius: 50%; }
    .dot-green { background: #10b981; box-shadow: 0 0 8px #10b981; }
    .dot-red { background: #ef4444; }

    /* Dashboard */
    .dashboard { display: grid; grid-template-columns: repeat(4, 1fr); gap: 10px; padding: 12px 16px; background: #141210; border-bottom: 1px solid #292524; }
    .card { background: #1c1917; border: 1px solid #292524; border-radius: 12px; padding: 10px; text-align: center; }
    .card-label { font-size: 0.65rem; color: #a8a29e; text-transform: uppercase; letter-spacing: 0.5px; }
    .card-value { font-size: 1.3rem; font-weight: 700; color: #f59e0b; margin-top: 2px; }
    .gear-val { font-size: 1.6rem; color: #38bdf8; }

    /* Main Area */
    main { flex: 1; display: grid; grid-template-columns: 1fr 1fr; gap: 16px; padding: 16px; height: 100%; }

    /* Controls Column 1: Joystick */
    .joystick-container { background: #1c1917; border: 1px solid #292524; border-radius: 20px; display: flex; flex-direction: column; align-items: center; justify-content: center; position: relative; }
    .joystick-base { width: 170px; height: 170px; background: #0c0a09; border: 2px solid #38bdf840; border-radius: 50%; position: relative; touch-action: none; display: flex; align-items: center; justify-content: center; }
    .joystick-handle { width: 60px; height: 60px; background: radial-gradient(circle, #38bdf8, #0284c7); border-radius: 50%; position: absolute; box-shadow: 0 0 15px #0284c780; cursor: pointer; }

    /* Controls Column 2: Panels */
    .panel-col { display: flex; flex-direction: column; gap: 12px; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    
    .btn { background: #292524; border: 1px solid #44403c; color: #f5f5f4; border-radius: 14px; padding: 14px 10px; font-weight: 700; font-size: 0.9rem; display: flex; align-items: center; justify-content: center; gap: 8px; cursor: pointer; transition: all 0.1s ease; }
    .btn:active { transform: scale(0.96); }
    .btn-active-amber { background: #d97706 !important; border-color: #f59e0b !important; color: #ffffff !important; box-shadow: 0 0 12px #f59e0b60; }
    .btn-active-red { background: #dc2626 !important; border-color: #ef4444 !important; color: #ffffff !important; box-shadow: 0 0 12px #ef444460; }
    .btn-gear { background: #0369a1; border-color: #38bdf8; font-size: 1.2rem; }
    .btn-brake { background: #991b1b; border-color: #f87171; grid-column: span 2; padding: 16px; font-size: 1.1rem; }
  </style>
</head>
<body>

  <header>
    <div class="logo">🚘 ToritoRC ⚡</div>
    <div id="status-badge" class="badge disconnected">
      <div id="status-dot" class="dot dot-red"></div>
      <span id="status-text">Desconectado</span>
    </div>
  </header>

  <div class="dashboard">
    <div class="card">
      <div class="card-label">Marcha</div>
      <div id="dash-gear" class="card-value gear-val">N</div>
    </div>
    <div class="card">
      <div class="card-label">RPM</div>
      <div id="dash-rpm" class="card-value">800</div>
    </div>
    <div class="card">
      <div class="card-label">Velocidad</div>
      <div id="dash-speed" class="card-value">0.0</div>
    </div>
    <div class="card">
      <div class="card-label">Batería</div>
      <div id="dash-battery" class="card-value">7.4V</div>
    </div>
  </div>

  <main>
    <div class="joystick-container">
      <div id="joy-base" class="joystick-base">
        <div id="joy-handle" class="joystick-handle"></div>
      </div>
      <div style="font-size: 0.75rem; color: #78716c; margin-top: 12px; font-weight: 600;">JOYSTICK DIRECCIÓN</div>
    </div>

    <div class="panel-col">
      <div class="btn-grid">
        <button id="btn-shift-down" class="btn btn-gear">⚙️ MARCH-</button>
        <button id="btn-shift-up" class="btn btn-gear">⚙️ MARCH+</button>
      </div>

      <div class="btn-grid">
        <button id="btn-lights" class="btn">💡 FAROS</button>
        <button id="btn-horn" class="btn">📢 BOCINA</button>
      </div>

      <div class="btn-grid">
        <button id="btn-left-blink" class="btn">◄ IZQUIERDA</button>
        <button id="btn-right-blink" class="btn">DERECHA ►</button>
      </div>

      <button id="btn-brake" class="btn btn-brake">🛑 FRENO DE MANO</button>
    </div>
  </main>

  <script>
    // State
    const cmd = {
      leftMotor: 0,
      rightMotor: 0,
      gear: 0,
      frontLights: false,
      leftBlinker: false,
      rightBlinker: false,
      brake: false,
      horn: false
    };

    let ws = null;
    let connected = false;

    // Audio Engine (Web Audio API)
    let audioCtx = null;
    let osc = null;
    let gain = null;

    function initAudio() {
      if (audioCtx) return;
      try {
        audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        osc = audioCtx.createOscillator();
        gain = audioCtx.createGain();
        osc.type = 'sawtooth';
        osc.frequency.setValueAtTime(40, audioCtx.currentTime);
        gain.gain.setValueAtTime(0.05, audioCtx.currentTime);
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        osc.start();
      } catch(e) {}
    }

    function updateAudio(rpm) {
      if (!audioCtx || !osc) return;
      const freq = 30 + (rpm / 100);
      osc.frequency.setTargetAtTime(freq, audioCtx.currentTime, 0.1);
    }

    // WebSocket Connection
    function connectWS() {
      ws = new WebSocket('ws://' + window.location.hostname + ':81');
      
      ws.onopen = () => {
        connected = true;
        document.getElementById('status-badge').className = 'badge connected';
        document.getElementById('status-dot').className = 'dot dot-green';
        document.getElementById('status-text').innerText = 'Conectado';
      };

      ws.onclose = () => {
        connected = false;
        document.getElementById('status-badge').className = 'badge disconnected';
        document.getElementById('status-dot').className = 'dot dot-red';
        document.getElementById('status-text').innerText = 'Desconectado';
        setTimeout(connectWS, 1500);
      };

      ws.onmessage = (e) => {
        try {
          const telemetry = JSON.parse(e.data);
          const gearNames = ['N', '1ª', '2ª', '3ª', '4ª', '5ª', 'R'];
          document.getElementById('dash-gear').innerText = gearNames[telemetry.gear] || 'N';
          document.getElementById('dash-rpm').innerText = telemetry.rpm || 800;
          document.getElementById('dash-speed').innerText = (telemetry.speed || 0).toFixed(1);
          document.getElementById('dash-battery').innerText = (telemetry.battery || 7.4).toFixed(1) + 'V';
          updateAudio(telemetry.rpm || 800);
        } catch(err) {}
      };
    }

    function sendCmd() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(cmd));
      }
    }

    // Joystick Logic
    const joyBase = document.getElementById('joy-base');
    const joyHandle = document.getElementById('joy-handle');
    let isDragging = false;

    function handleJoystick(x, y) {
      const rect = joyBase.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const centerY = rect.top + rect.height / 2;
      
      let deltaX = x - centerX;
      let deltaY = y - centerY;
      const maxRadius = rect.width / 2 - 30;
      const dist = Math.hypot(deltaX, deltaY);

      if (dist > maxRadius) {
        deltaX = (deltaX / dist) * maxRadius;
        deltaY = (deltaY / dist) * maxRadius;
      }

      joyHandle.style.transform = `translate(${deltaX}px, ${deltaY}px)`;

      // Normalizar -255 a 255
      const normY = -Math.round((deltaY / maxRadius) * 255);
      const normX = Math.round((deltaX / maxRadius) * 255);

      // Mezcla diferencial de motores
      let left = normY + normX;
      let right = normY - normX;

      cmd.leftMotor = Math.max(-255, Math.min(255, left));
      cmd.rightMotor = Math.max(-255, Math.min(255, right));

      sendCmd();
    }

    function resetJoystick() {
      joyHandle.style.transform = 'translate(0px, 0px)';
      cmd.leftMotor = 0;
      cmd.rightMotor = 0;
      sendCmd();
    }

    joyBase.addEventListener('pointerdown', (e) => { isDragging = true; initAudio(); handleJoystick(e.clientX, e.clientY); });
    window.addEventListener('pointermove', (e) => { if (isDragging) handleJoystick(e.clientX, e.clientY); });
    window.addEventListener('pointerup', () => { isDragging = false; resetJoystick(); });

    // Buttons Setup
    document.getElementById('btn-lights').onclick = function() {
      cmd.frontLights = !cmd.frontLights;
      this.classList.toggle('btn-active-amber', cmd.frontLights);
      sendCmd();
    };

    document.getElementById('btn-left-blink').onclick = function() {
      cmd.leftBlinker = !cmd.leftBlinker;
      if (cmd.leftBlinker) cmd.rightBlinker = false;
      document.getElementById('btn-right-blink').classList.remove('btn-active-amber');
      this.classList.toggle('btn-active-amber', cmd.leftBlinker);
      sendCmd();
    };

    document.getElementById('btn-right-blink').onclick = function() {
      cmd.rightBlinker = !cmd.rightBlinker;
      if (cmd.rightBlinker) cmd.leftBlinker = false;
      document.getElementById('btn-left-blink').classList.remove('btn-active-amber');
      this.classList.toggle('btn-active-amber', cmd.rightBlinker);
      sendCmd();
    };

    document.getElementById('btn-shift-up').onclick = () => {
      if (cmd.gear < 6) cmd.gear++;
      sendCmd();
    };

    document.getElementById('btn-shift-down').onclick = () => {
      if (cmd.gear > 0) cmd.gear--;
      sendCmd();
    };

    const brakeBtn = document.getElementById('btn-brake');
    brakeBtn.onpointerdown = () => { cmd.brake = true; brakeBtn.classList.add('btn-active-red'); sendCmd(); };
    brakeBtn.onpointerup = () => { cmd.brake = false; brakeBtn.classList.remove('btn-active-red'); sendCmd(); };

    const hornBtn = document.getElementById('btn-horn');
    hornBtn.onpointerdown = () => { cmd.horn = true; hornBtn.classList.add('btn-active-amber'); sendCmd(); };
    hornBtn.onpointerup = () => { cmd.horn = false; hornBtn.classList.remove('btn-active-amber'); sendCmd(); };

    // Start
    connectWS();
  </script>
</body>
</html>
)rawliteral";

// ============================================================================
// SETUP & LOOP PRINCIPAL
// ============================================================================
void setup() {
  Serial.begin(115200);

  // Configuración de Pines de Salida
  pinMode(PIN_FAROS, OUTPUT);
  pinMode(PIN_INT_IZQ, OUTPUT);
  pinMode(PIN_INT_DER, OUTPUT);
  pinMode(PIN_FRENO, OUTPUT);
  pinMode(PIN_SPEAKER, OUTPUT);

  pinMode(PIN_MOTOR_IZQ_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DER_PWM, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_IN2, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN3, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN4, OUTPUT);

  // Estado inicial apagado
  digitalWrite(PIN_FAROS, LOW);
  digitalWrite(PIN_INT_IZQ, LOW);
  digitalWrite(PIN_INT_DER, LOW);
  digitalWrite(PIN_FRENO, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

  // Configuración de WiFi Access Point (IP Fija 192.168.4.1)
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("ToritoRC", "toritopass");

  // Servidor Web HTTP (Entrega la página en http://192.168.4.1)
  server.on("/", []() {
    server.send(200, "text/html", HTML_INDEX);
  });
  server.begin();

  // Servidor WebSockets (Canal de comandos y telemetría en ws://192.168.4.1:81)
  ws.begin();
  ws.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
      payload[length] = 0;
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, (char*)payload) == DeserializationError::Ok) {
        car.leftMotor    = doc["leftMotor"] | 0;
        car.rightMotor   = doc["rightMotor"] | 0;
        car.gear         = doc["gear"] | 0;
        car.frontLights  = doc["frontLights"] | false;
        car.leftBlinker  = doc["leftBlinker"] | false;
        car.rightBlinker = doc["rightBlinker"] | false;
        car.brake        = doc["brake"] | false;
        car.horn         = doc["horn"] | false;
      }
    }
  });

  Serial.println("ToritoRC Listo!");
}

void loop() {
  unsigned long now = millis();

  // 1. Manejo de Servidores
  server.handleClient();
  ws.loop();

  // 2. Control de Motores (L298N)
  float speedFactor = GEAR_RATIOS[car.gear];
  int leftPwm = abs(car.leftMotor) * speedFactor;
  int rightPwm = abs(car.rightMotor) * speedFactor;

  analogWrite(PIN_MOTOR_IZQ_PWM, leftPwm);
  analogWrite(PIN_MOTOR_DER_PWM, rightPwm);

  // Dirección Motor Izquierdo
  if (car.leftMotor > 0) {
    digitalWrite(PIN_MOTOR_IZQ_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IZQ_IN2, LOW);
  } else if (car.leftMotor < 0) {
    digitalWrite(PIN_MOTOR_IZQ_IN1, LOW);
    digitalWrite(PIN_MOTOR_IZQ_IN2, HIGH);
  } else {
    digitalWrite(PIN_MOTOR_IZQ_IN1, LOW);
    digitalWrite(PIN_MOTOR_IZQ_IN2, LOW);
  }

  // Dirección Motor Derecho
  if (car.rightMotor > 0) {
    digitalWrite(PIN_MOTOR_DER_IN3, HIGH);
    digitalWrite(PIN_MOTOR_DER_IN4, LOW);
  } else if (car.rightMotor < 0) {
    digitalWrite(PIN_MOTOR_DER_IN3, LOW);
    digitalWrite(PIN_MOTOR_DER_IN4, HIGH);
  } else {
    digitalWrite(PIN_MOTOR_DER_IN3, LOW);
    digitalWrite(PIN_MOTOR_DER_IN4, LOW);
  }

  // 3. Control de Luces
  digitalWrite(PIN_FAROS, car.frontLights ? HIGH : LOW);
  digitalWrite(PIN_FRENO, car.brake ? HIGH : LOW);

  // Destello de Intermitentes (2 Hz = 250ms)
  if (now - lastBlinkerToggle >= 250) {
    lastBlinkerToggle = now;
    blinkerState = !blinkerState;
  }
  digitalWrite(PIN_INT_IZQ, (car.leftBlinker && blinkerState) ? HIGH : LOW);
  digitalWrite(PIN_INT_DER, (car.rightBlinker && blinkerState) ? HIGH : LOW);

  // 4. Sonido de Bocina en ESP8266
  if (car.horn) {
    tone(PIN_SPEAKER, 440); // 440 Hz
  } else {
    noTone(PIN_SPEAKER);
  }

  // 5. Cálculo y Envío de Telemetría (10 Hz = 100ms)
  if (now - lastTelemetrySend >= 100) {
    lastTelemetrySend = now;

    int maxThrottle = max(abs(car.leftMotor), abs(car.rightMotor));
    car.rpm = 800 + (maxThrottle * 15 * speedFactor);
    car.speedMps = (maxThrottle / 255.0f) * 3.5f * speedFactor;
    car.batteryVoltage = analogRead(PIN_BATTERY) * (3.3f / 1024.0f) * 2.5f;

    StaticJsonDocument<256> doc;
    doc["battery"] = car.batteryVoltage;
    doc["speed"]   = car.speedMps;
    doc["gear"]    = car.gear;
    doc["rpm"]     = car.rpm;
    doc["frontLights"]  = car.frontLights;
    doc["leftBlinker"]  = car.leftBlinker;
    doc["rightBlinker"] = car.rightBlinker;
    doc["brake"]        = car.brake;

    String json;
    serializeJson(doc, json);
    ws.broadcastTXT(json);
  }
}
