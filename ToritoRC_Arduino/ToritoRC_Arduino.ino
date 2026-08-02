#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// ============================================================
// AUTITO RC - Wemos D1 Mini ESP8266 + L298N
// Control exclusivo mediante joystick web
// Red Wi-Fi: autito-rc
// Clave:     12345678
// Web:       http://192.168.4.1
// ============================================================

#define PIN_MOTOR_IZQ_PWM  D5  // GPIO14 - ENA
#define PIN_MOTOR_DER_PWM  D6  // GPIO12 - ENB
#define PIN_MOTOR_IZQ_IN1  D7  // GPIO13 - IN1
#define PIN_MOTOR_IZQ_IN2  D8  // GPIO15 - IN2
#define PIN_MOTOR_DER_IN3  RX  // GPIO3  - IN3
#define PIN_MOTOR_DER_IN4  TX  // GPIO1  - IN4

const char* AP_SSID = "autito-rc";
const char* AP_PASSWORD = "12345678";

IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
ESP8266WebServer server(80);

constexpr uint16_t DNS_PORT = 53;
constexpr uint32_t COMMAND_TIMEOUT_MS = 450;
constexpr uint32_t RAMP_INTERVAL_MS = 10;
constexpr int PWM_MAX = 255;
constexpr int RAMP_STEP = 42;  // Aceleración rápida: 0 -> 255 en unos 60 ms.

int targetLeft = 0;
int targetRight = 0;
int currentLeft = 0;
int currentRight = 0;
uint32_t lastCommandMs = 0;
uint32_t lastRampMs = 0;
bool commandActive = false;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
  <meta name="theme-color" content="#07111f">
  <title>Autito RC</title>
  <style>
    :root {
      --bg1:#06101d;
      --bg2:#102b49;
      --glass:rgba(255,255,255,.085);
      --line:rgba(255,255,255,.14);
      --text:#f7fbff;
      --muted:#9cb3ca;
      --cyan:#48ddff;
      --blue:#4e73ff;
      --green:#48e6aa;
      --danger:#ff6079;
    }

    * { box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
    html,body { margin:0; width:100%; height:100%; overflow:hidden; overscroll-behavior:none; }
    body {
      font-family:Inter,system-ui,-apple-system,"Segoe UI",sans-serif;
      color:var(--text);
      background:
        radial-gradient(circle at 10% 5%,rgba(72,221,255,.20),transparent 30%),
        radial-gradient(circle at 92% 92%,rgba(78,115,255,.24),transparent 34%),
        linear-gradient(145deg,var(--bg1),var(--bg2));
      user-select:none;
      touch-action:none;
    }

    body::before {
      content:"";
      position:fixed;
      inset:-45%;
      background:conic-gradient(from 20deg,transparent,rgba(72,221,255,.075),transparent 30%);
      animation:spin 17s linear infinite;
      pointer-events:none;
    }
    @keyframes spin { to { transform:rotate(360deg); } }

    .app {
      position:relative;
      z-index:1;
      width:min(100%,520px);
      height:100dvh;
      margin:auto;
      padding:max(17px,env(safe-area-inset-top)) 18px max(17px,env(safe-area-inset-bottom));
      display:flex;
      flex-direction:column;
      gap:14px;
    }

    header { display:flex; align-items:center; justify-content:space-between; }
    .brand { display:flex; align-items:center; gap:11px; }
    .logo {
      width:46px; height:46px; border-radius:15px;
      display:grid; place-items:center; font-size:23px;
      background:linear-gradient(145deg,rgba(72,221,255,.27),rgba(78,115,255,.18));
      border:1px solid rgba(255,255,255,.18);
      box-shadow:0 12px 30px rgba(72,221,255,.13),inset 0 1px rgba(255,255,255,.2);
    }
    h1 { margin:0; font-size:19px; line-height:1; letter-spacing:.2px; }
    .subtitle { margin-top:5px; color:var(--muted); font-size:12px; }
    .connection {
      display:flex; align-items:center; gap:7px;
      padding:8px 10px; border-radius:999px;
      color:var(--muted); font-size:12px;
      background:var(--glass); border:1px solid var(--line);
    }
    .dot { width:8px; height:8px; border-radius:50%; background:var(--green); box-shadow:0 0 14px var(--green); }
    .dot.off { background:var(--danger); box-shadow:0 0 14px var(--danger); }

    .panel {
      flex:1; min-height:0;
      display:flex; flex-direction:column;
      padding:16px;
      border-radius:29px;
      background:linear-gradient(145deg,rgba(255,255,255,.105),rgba(255,255,255,.042));
      border:1px solid var(--line);
      box-shadow:0 24px 70px rgba(0,0,0,.38),inset 0 1px rgba(255,255,255,.11);
      backdrop-filter:blur(18px);
    }

    .title-row { display:flex; align-items:center; justify-content:space-between; padding:1px 3px 9px; }
    .mode-title { font-size:14px; font-weight:800; }
    .badge {
      padding:6px 9px; border-radius:999px;
      color:var(--cyan); font-size:10px; font-weight:800; letter-spacing:.7px;
      background:rgba(72,221,255,.08); border:1px solid rgba(72,221,255,.18);
    }

    .joystick-stage { flex:1; min-height:0; display:grid; place-items:center; }
    .joystick {
      position:relative;
      width:min(72vw,300px); height:min(72vw,300px);
      max-width:300px; max-height:300px;
      border-radius:50%;
      background:
        radial-gradient(circle at center,rgba(72,221,255,.13),rgba(0,0,0,.14) 62%),
        linear-gradient(145deg,rgba(255,255,255,.085),rgba(255,255,255,.018));
      border:1px solid rgba(255,255,255,.15);
      box-shadow:inset 0 0 42px rgba(0,0,0,.34),0 26px 58px rgba(0,0,0,.27);
      touch-action:none;
    }
    .joystick::before,.joystick::after { content:""; position:absolute; opacity:.19; }
    .joystick::before { left:12%; right:12%; top:50%; height:1px; background:white; }
    .joystick::after { top:12%; bottom:12%; left:50%; width:1px; background:white; }

    .ring {
      position:absolute; inset:18%; border-radius:50%;
      border:1px dashed rgba(255,255,255,.14);
      pointer-events:none;
    }
    .dir { position:absolute; color:rgba(255,255,255,.29); font-size:19px; font-weight:900; pointer-events:none; }
    .dir.up { top:16px; left:50%; transform:translateX(-50%); }
    .dir.down { bottom:16px; left:50%; transform:translateX(-50%); }
    .dir.left { left:18px; top:50%; transform:translateY(-50%); }
    .dir.right { right:18px; top:50%; transform:translateY(-50%); }

    .stick {
      position:absolute; left:50%; top:50%;
      width:96px; height:96px; margin:-48px;
      border-radius:50%;
      display:grid; place-items:center;
      font-size:23px;
      color:white;
      background:linear-gradient(145deg,rgba(104,232,255,.98),rgba(65,91,255,.98));
      border:2px solid rgba(255,255,255,.43);
      box-shadow:0 14px 32px rgba(17,94,205,.46),inset 0 5px 15px rgba(255,255,255,.27);
      transition:transform .08s ease-out;
      pointer-events:none;
    }
    .stick.dragging { transition:none; }

    .readout { display:grid; grid-template-columns:1fr auto 1fr; gap:10px; align-items:center; }
    .metric {
      padding:11px 12px; border-radius:15px;
      background:rgba(0,0,0,.16); border:1px solid rgba(255,255,255,.08);
    }
    .metric:last-child { text-align:right; }
    .metric small { display:block; color:var(--muted); font-size:10px; text-transform:uppercase; letter-spacing:.8px; }
    .metric strong { font-size:17px; }

    .speed-ring {
      position:relative;
      width:74px; height:74px; border-radius:50%;
      display:grid; place-items:center;
      background:conic-gradient(var(--cyan) var(--speed,0%),rgba(255,255,255,.08) 0);
      box-shadow:0 0 24px rgba(72,221,255,.13);
    }
    .speed-ring::before { content:""; position:absolute; width:58px; height:58px; border-radius:50%; background:#10243b; }
    .speed-ring span { position:relative; font-size:13px; font-weight:850; }

    .hint { margin:11px 4px 0; color:var(--muted); font-size:11px; line-height:1.45; text-align:center; }
    .footer { display:flex; justify-content:center; align-items:center; gap:8px; color:var(--muted); font-size:11px; }
    .mini-dot { width:6px; height:6px; border-radius:50%; background:var(--green); }

    @media (max-height:680px) {
      .app { padding-top:10px; padding-bottom:10px; gap:9px; }
      .panel { padding:12px; border-radius:24px; }
      .joystick { width:230px; height:230px; }
      .stick { width:82px; height:82px; margin:-41px; }
      .title-row { padding-bottom:3px; }
    }
  </style>
</head>
<body>
  <main class="app">
    <header>
      <div class="brand">
        <div class="logo">🏎️</div>
        <div>
          <h1>Autito RC</h1>
          <div class="subtitle">Control Wi-Fi directo</div>
        </div>
      </div>
      <div class="connection"><span id="netDot" class="dot"></span><span id="netText">Conectado</span></div>
    </header>

    <section class="panel">
      <div class="title-row">
        <span class="mode-title">Joystick proporcional</span>
        <span class="badge">MODO TANQUE</span>
      </div>

      <div class="joystick-stage">
        <div id="joystick" class="joystick">
          <div class="ring"></div>
          <span class="dir up">▲</span>
          <span class="dir down">▼</span>
          <span class="dir left">◀</span>
          <span class="dir right">▶</span>
          <div id="stick" class="stick">✦</div>
        </div>
      </div>

      <div class="readout">
        <div class="metric"><small>Movimiento</small><strong id="movement">Detenido</strong></div>
        <div id="speedRing" class="speed-ring"><span id="speedText">0%</span></div>
        <div class="metric"><small>Potencia</small><strong id="power">0</strong></div>
      </div>

      <div class="hint">Arrastra el control en una dirección. Al soltarlo, el auto se detiene inmediatamente.</div>
    </section>

    <div class="footer"><span class="mini-dot"></span> Parada automática activa si se pierde la señal</div>
  </main>

<script>
(() => {
  'use strict';

  const MAX_PWM = 255;
  const SEND_INTERVAL_MS = 80;
  const JOYSTICK_DEADZONE = 0.08;

  let desiredLeft = 0;
  let desiredRight = 0;
  let lastSentLeft = null;
  let lastSentRight = null;
  let requestPending = false;
  let joyPointer = null;

  const $ = id => document.getElementById(id);
  const joystick = $('joystick');
  const stick = $('stick');
  const clamp = (v,min,max) => Math.max(min,Math.min(max,v));

  function setNetwork(ok) {
    $('netDot').classList.toggle('off',!ok);
    $('netText').textContent = ok ? 'Conectado' : 'Sin señal';
  }

  function movementLabel(throttle,turn) {
    if (Math.abs(throttle) < .02 && Math.abs(turn) < .02) return 'Detenido';
    if (Math.abs(throttle) >= Math.abs(turn)) return throttle > 0 ? 'Adelante' : 'Atrás';
    return turn > 0 ? 'Derecha' : 'Izquierda';
  }

  function updateReadout(throttle,turn) {
    const intensity = Math.round(Math.max(Math.abs(throttle),Math.abs(turn)) * 100);
    $('movement').textContent = movementLabel(throttle,turn);
    $('power').textContent = Math.round(intensity * MAX_PWM / 100);
    $('speedText').textContent = intensity + '%';
    $('speedRing').style.setProperty('--speed',intensity + '%');
  }

  function driveFromAxes(throttle,turn) {
    throttle = clamp(throttle,-1,1);
    turn = clamp(turn,-1,1);

    if (Math.abs(throttle) < JOYSTICK_DEADZONE) throttle = 0;
    if (Math.abs(turn) < JOYSTICK_DEADZONE) turn = 0;

    // No se permiten diagonales: se conserva solamente el eje dominante.
    if (Math.abs(throttle) >= Math.abs(turn)) turn = 0;
    else throttle = 0;

    let left = 0;
    let right = 0;

    if (throttle !== 0) {
      // Adelante/atrás invertidos respecto a la versión anterior.
      // Empujar hacia arriba sigue mostrando "Adelante", pero invierte
      // la polaridad de ambos motores para corregir su orientación física.
      left = -throttle;
      right = -throttle;
    } else if (turn !== 0) {
      // Giro tipo tanque. Se mantiene igual que en la versión anterior.
      left = turn;
      right = -turn;
    }

    desiredLeft = Math.round(left * MAX_PWM);
    desiredRight = Math.round(right * MAX_PWM);
    updateReadout(throttle,turn);
  }

  function stopNow() {
    desiredLeft = 0;
    desiredRight = 0;
    updateReadout(0,0);
    fetch('/stop',{cache:'no-store',keepalive:true}).catch(() => {});
  }

  async function transmit() {
    const changed = desiredLeft !== lastSentLeft || desiredRight !== lastSentRight;
    const moving = desiredLeft !== 0 || desiredRight !== 0;
    if ((!changed && !moving) || requestPending) return;

    requestPending = true;
    try {
      const response = await fetch(`/cmd?l=${desiredLeft}&r=${desiredRight}&t=${Date.now()}`,{cache:'no-store'});
      if (!response.ok) throw new Error('HTTP ' + response.status);
      lastSentLeft = desiredLeft;
      lastSentRight = desiredRight;
      setNetwork(true);
    } catch (error) {
      setNetwork(false);
    } finally {
      requestPending = false;
    }
  }
  setInterval(transmit,SEND_INTERVAL_MS);

  function updateJoystick(event) {
    const rect = joystick.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    const maxRadius = rect.width * .31;

    let dx = event.clientX - centerX;
    let dy = event.clientY - centerY;
    const distance = Math.hypot(dx,dy);

    if (distance > maxRadius) {
      dx = dx / distance * maxRadius;
      dy = dy / distance * maxRadius;
    }

    let x = dx / maxRadius;
    let y = -dy / maxRadius;

    // Un solo eje: vertical u horizontal.
    if (Math.abs(y) >= Math.abs(x)) x = 0;
    else y = 0;

    stick.style.transform = `translate(${x * maxRadius}px,${-y * maxRadius}px)`;
    driveFromAxes(y,x);
  }

  function resetJoystick() {
    joyPointer = null;
    stick.classList.remove('dragging');
    stick.style.transform = 'translate(0,0)';
    driveFromAxes(0,0);
  }

  joystick.addEventListener('pointerdown',event => {
    joyPointer = event.pointerId;
    joystick.setPointerCapture(event.pointerId);
    stick.classList.add('dragging');
    updateJoystick(event);
  });

  joystick.addEventListener('pointermove',event => {
    if (event.pointerId === joyPointer) updateJoystick(event);
  });

  ['pointerup','pointercancel','lostpointercapture'].forEach(type => {
    joystick.addEventListener(type,event => {
      if (joyPointer === null || event.pointerId === joyPointer) {
        resetJoystick();
        stopNow();
      }
    });
  });

  document.addEventListener('visibilitychange',() => {
    if (document.hidden) stopNow();
  });
  window.addEventListener('pagehide',stopNow);
  window.addEventListener('blur',stopNow);
  window.addEventListener('orientationchange',stopNow);

  document.addEventListener('contextmenu',event => event.preventDefault());
  document.addEventListener('touchmove',event => event.preventDefault(),{passive:false});
})();
</script>
</body>
</html>
)rawliteral";

int approachValue(int current, int target, int step) {
  if (current < target) return min(current + step, target);
  if (current > target) return max(current - step, target);
  return current;
}

void setSingleMotor(uint8_t pwmPin, uint8_t pinA, uint8_t pinB, int speedValue) {
  speedValue = constrain(speedValue, -PWM_MAX, PWM_MAX);

  if (speedValue == 0) {
    analogWrite(pwmPin, 0);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    return;
  }

  // Primero establece la dirección y después aplica el PWM.
  if (speedValue > 0) {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
  } else {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
  }

  analogWrite(pwmPin, abs(speedValue));
}

void applyMotors() {
  setSingleMotor(PIN_MOTOR_IZQ_PWM, PIN_MOTOR_IZQ_IN1, PIN_MOTOR_IZQ_IN2, currentLeft);
  setSingleMotor(PIN_MOTOR_DER_PWM, PIN_MOTOR_DER_IN3, PIN_MOTOR_DER_IN4, currentRight);
}

void hardStop() {
  targetLeft = 0;
  targetRight = 0;
  currentLeft = 0;
  currentRight = 0;
  commandActive = false;
  applyMotors();
}

void updateRamp() {
  const uint32_t now = millis();
  if (now - lastRampMs < RAMP_INTERVAL_MS) return;
  lastRampMs = now;

  // Una orden cero detiene inmediatamente el auto.
  if (targetLeft == 0 && targetRight == 0) {
    if (currentLeft != 0 || currentRight != 0) hardStop();
    return;
  }

  currentLeft = approachValue(currentLeft, targetLeft, RAMP_STEP);
  currentRight = approachValue(currentRight, targetRight, RAMP_STEP);
  applyMotors();
}

void addNoCacheHeaders() {
  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

void handleRoot() {
  addNoCacheHeaders();
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleCommand() {
  if (!server.hasArg("l") || !server.hasArg("r")) {
    server.send(400, "text/plain", "Faltan parametros l y r");
    return;
  }

  const int left = constrain(server.arg("l").toInt(), -PWM_MAX, PWM_MAX);
  const int right = constrain(server.arg("r").toInt(), -PWM_MAX, PWM_MAX);

  targetLeft = abs(left) < 3 ? 0 : left;
  targetRight = abs(right) < 3 ? 0 : right;
  lastCommandMs = millis();
  commandActive = true;

  if (targetLeft == 0 && targetRight == 0) hardStop();

  addNoCacheHeaders();
  server.send(204, "text/plain", "");
}

void handleStop() {
  hardStop();
  addNoCacheHeaders();
  server.send(204, "text/plain", "");
}

void handleStatus() {
  String json = "{\"clients\":" + String(WiFi.softAPgetStationNum()) +
                ",\"left\":" + String(currentLeft) +
                ",\"right\":" + String(currentRight) + "}";
  addNoCacheHeaders();
  server.send(200, "application/json", json);
}

void setup() {
  // GPIO1/TX y GPIO3/RX controlan el motor derecho; no se inicia Serial.
  pinMode(PIN_MOTOR_IZQ_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DER_PWM, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IZQ_IN2, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN3, OUTPUT);
  pinMode(PIN_MOTOR_DER_IN4, OUTPUT);

  analogWriteRange(PWM_MAX);
  analogWriteFreq(1000);
  hardStop();

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD, 6, false, 4);

  // Portal cautivo: cualquier dominio apunta al autito.
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/status", HTTP_GET, handleStatus);

  // Rutas consultadas por Android, iOS y Windows para detectar portales cautivos.
  server.on("/generate_204", HTTP_GET, handleRoot);
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
  server.on("/fwlink", HTTP_GET, handleRoot);
  server.onNotFound(handleRoot);

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (commandActive && millis() - lastCommandMs > COMMAND_TIMEOUT_MS) {
    hardStop();
  } else {
    updateRamp();
  }

  yield();
}
