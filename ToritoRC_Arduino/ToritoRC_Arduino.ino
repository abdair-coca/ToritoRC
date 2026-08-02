#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// ============================================================
// AUTITO RC - Wemos D1 Mini ESP8266 + L298N
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
constexpr int RAMP_STEP = 42;  // 0 -> 255 en ~60 ms

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
      --bg-1:#07111f;
      --bg-2:#102743;
      --glass:rgba(255,255,255,.09);
      --line:rgba(255,255,255,.14);
      --text:#f7fbff;
      --muted:#9eb3c9;
      --cyan:#49d8ff;
      --blue:#4c7dff;
      --green:#45e6a8;
      --danger:#ff6178;
      --shadow:0 22px 70px rgba(0,0,0,.38);
    }

    * { box-sizing:border-box; -webkit-tap-highlight-color:transparent; }
    html, body { margin:0; min-height:100%; overflow:hidden; overscroll-behavior:none; }
    body {
      font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;
      color:var(--text);
      background:
        radial-gradient(circle at 12% 5%, rgba(73,216,255,.18), transparent 32%),
        radial-gradient(circle at 90% 92%, rgba(76,125,255,.22), transparent 36%),
        linear-gradient(145deg,var(--bg-1),var(--bg-2));
      touch-action:none;
      user-select:none;
    }

    body::before {
      content:"";
      position:fixed;
      inset:-30%;
      background:conic-gradient(from 20deg,transparent,rgba(73,216,255,.08),transparent 30%);
      animation:drift 16s linear infinite;
      pointer-events:none;
    }
    @keyframes drift { to { transform:rotate(360deg); } }

    .app {
      position:relative;
      z-index:1;
      width:min(100%,520px);
      height:100dvh;
      margin:auto;
      padding:max(18px,env(safe-area-inset-top)) 18px max(18px,env(safe-area-inset-bottom));
      display:flex;
      flex-direction:column;
      gap:14px;
    }

    header { display:flex; align-items:center; justify-content:space-between; }
    .brand { display:flex; align-items:center; gap:11px; }
    .logo {
      width:45px; height:45px; border-radius:15px;
      display:grid; place-items:center;
      font-size:23px;
      background:linear-gradient(145deg,rgba(73,216,255,.28),rgba(76,125,255,.18));
      border:1px solid rgba(255,255,255,.18);
      box-shadow:0 10px 30px rgba(73,216,255,.13), inset 0 1px rgba(255,255,255,.2);
    }
    h1 { font-size:18px; line-height:1.05; margin:0; letter-spacing:.2px; }
    .subtitle { color:var(--muted); font-size:12px; margin-top:4px; }
    .connection {
      display:flex; align-items:center; gap:7px;
      color:var(--muted); font-size:12px;
      padding:8px 10px; border-radius:999px;
      background:var(--glass); border:1px solid var(--line);
    }
    .dot { width:8px; height:8px; border-radius:50%; background:var(--green); box-shadow:0 0 14px var(--green); }
    .dot.off { background:var(--danger); box-shadow:0 0 14px var(--danger); }

    .panel {
      flex:1;
      min-height:0;
      display:flex;
      flex-direction:column;
      padding:14px;
      border-radius:28px;
      background:linear-gradient(145deg,rgba(255,255,255,.105),rgba(255,255,255,.045));
      border:1px solid var(--line);
      box-shadow:var(--shadow), inset 0 1px rgba(255,255,255,.11);
      backdrop-filter:blur(18px);
    }

    .tabs {
      display:grid; grid-template-columns:1fr 1fr;
      gap:5px; padding:5px;
      background:rgba(0,0,0,.18);
      border:1px solid rgba(255,255,255,.08);
      border-radius:16px;
    }
    .tab {
      border:0; border-radius:12px; padding:11px 8px;
      color:var(--muted); background:transparent;
      font-weight:750; font-size:13px;
    }
    .tab.active {
      color:white;
      background:linear-gradient(135deg,rgba(73,216,255,.23),rgba(76,125,255,.27));
      box-shadow:inset 0 1px rgba(255,255,255,.16),0 8px 18px rgba(0,0,0,.15);
    }

    .mode { flex:1; min-height:0; display:none; align-items:center; justify-content:center; flex-direction:column; }
    .mode.active { display:flex; }

    .tilt-stage { width:100%; flex:1; display:grid; place-items:center; perspective:800px; }
    .phone-wrap { position:relative; width:185px; height:245px; display:grid; place-items:center; }
    .halo {
      position:absolute; width:230px; height:230px; border-radius:50%;
      background:radial-gradient(circle,rgba(73,216,255,.2),rgba(73,216,255,.02) 55%,transparent 70%);
      filter:blur(2px); animation:pulse 2.4s ease-in-out infinite;
    }
    @keyframes pulse { 50% { transform:scale(1.08); opacity:.72; } }
    .phone {
      position:relative;
      width:112px; height:210px;
      border-radius:25px;
      border:2px solid rgba(255,255,255,.66);
      background:linear-gradient(145deg,rgba(255,255,255,.16),rgba(255,255,255,.05));
      box-shadow:0 25px 45px rgba(0,0,0,.35),inset 0 0 25px rgba(73,216,255,.09);
      transform-style:preserve-3d;
      transition:transform .08s linear;
    }
    .phone::before { content:""; position:absolute; top:8px; left:37px; width:34px; height:5px; border-radius:5px; background:rgba(255,255,255,.45); }
    .phone::after { content:""; position:absolute; bottom:10px; left:48px; width:13px; height:13px; border:1px solid rgba(255,255,255,.42); border-radius:50%; }
    .cross { position:absolute; inset:35px 14px; }
    .cross::before,.cross::after { content:""; position:absolute; background:linear-gradient(90deg,transparent,rgba(73,216,255,.65),transparent); }
    .cross::before { left:0; right:0; top:50%; height:1px; }
    .cross::after { top:0; bottom:0; left:50%; width:1px; background:linear-gradient(transparent,rgba(73,216,255,.65),transparent); }
    .arrow { font-size:36px; filter:drop-shadow(0 0 14px rgba(73,216,255,.8)); transition:transform .1s,opacity .1s; }

    .readout { width:100%; display:grid; grid-template-columns:1fr auto 1fr; gap:10px; align-items:center; }
    .metric { padding:11px 12px; border-radius:15px; background:rgba(0,0,0,.16); border:1px solid rgba(255,255,255,.08); }
    .metric:last-child { text-align:right; }
    .metric small { display:block; color:var(--muted); font-size:10px; text-transform:uppercase; letter-spacing:.8px; }
    .metric strong { font-size:17px; }
    .speed-ring {
      width:72px; height:72px; border-radius:50%; display:grid; place-items:center;
      background:conic-gradient(var(--cyan) var(--speed,0%),rgba(255,255,255,.08) 0);
      box-shadow:0 0 22px rgba(73,216,255,.13);
    }
    .speed-ring::before { content:""; width:57px; height:57px; border-radius:50%; background:#10233a; position:absolute; }
    .speed-ring span { position:relative; font-size:13px; font-weight:800; }

    .primary {
      width:100%; margin-top:12px; padding:14px;
      border:0; border-radius:16px; color:white;
      font-size:14px; font-weight:850;
      background:linear-gradient(135deg,var(--cyan),var(--blue));
      box-shadow:0 14px 28px rgba(49,133,255,.24),inset 0 1px rgba(255,255,255,.45);
    }
    .primary:active { transform:scale(.985); }

    .hint { min-height:34px; margin:9px 4px 0; color:var(--muted); font-size:11px; line-height:1.45; text-align:center; }

    .joystick-stage { flex:1; width:100%; display:grid; place-items:center; }
    .joystick {
      position:relative;
      width:min(68vw,285px); height:min(68vw,285px); max-width:285px; max-height:285px;
      border-radius:50%;
      background:
        radial-gradient(circle at center,rgba(73,216,255,.12),rgba(0,0,0,.14) 62%),
        linear-gradient(145deg,rgba(255,255,255,.08),rgba(255,255,255,.02));
      border:1px solid rgba(255,255,255,.14);
      box-shadow:inset 0 0 38px rgba(0,0,0,.32),0 25px 55px rgba(0,0,0,.24);
      touch-action:none;
    }
    .joystick::before,.joystick::after { content:""; position:absolute; opacity:.22; }
    .joystick::before { left:12%; right:12%; top:50%; height:1px; background:white; }
    .joystick::after { top:12%; bottom:12%; left:50%; width:1px; background:white; }
    .dir { position:absolute; color:rgba(255,255,255,.25); font-size:18px; font-weight:900; }
    .dir.up { top:16px; left:50%; transform:translateX(-50%); }
    .dir.down { bottom:16px; left:50%; transform:translateX(-50%); }
    .dir.left { left:18px; top:50%; transform:translateY(-50%); }
    .dir.right { right:18px; top:50%; transform:translateY(-50%); }
    .stick {
      position:absolute; left:50%; top:50%;
      width:94px; height:94px; margin:-47px;
      border-radius:50%;
      background:linear-gradient(145deg,rgba(104,226,255,.95),rgba(65,98,255,.95));
      border:2px solid rgba(255,255,255,.42);
      box-shadow:0 13px 30px rgba(17,94,205,.45),inset 0 5px 15px rgba(255,255,255,.26);
      display:grid; place-items:center;
      font-size:22px;
      transition:transform .08s ease-out;
    }
    .stick.dragging { transition:none; }

    .footer-status { display:flex; justify-content:center; align-items:center; gap:8px; color:var(--muted); font-size:11px; }
    .mini-dot { width:6px; height:6px; border-radius:50%; background:var(--green); }

    @media (max-height:690px) {
      .app { gap:9px; padding-top:10px; padding-bottom:10px; }
      .panel { border-radius:23px; padding:11px; }
      .phone-wrap { transform:scale(.79); height:196px; }
      .tilt-stage { min-height:205px; }
      .readout { margin-top:-8px; }
      .joystick { width:230px; height:230px; }
    }
  </style>
</head>
<body>
  <main class="app">
    <header>
      <div class="brand">
        <div class="logo">🏎️</div>
        <div><h1>Autito RC</h1><div class="subtitle">Control Wi‑Fi directo</div></div>
      </div>
      <div class="connection"><span id="netDot" class="dot"></span><span id="netText">Conectado</span></div>
    </header>

    <section class="panel">
      <div class="tabs">
        <button class="tab active" data-mode="tilt">Inclinación</button>
        <button class="tab" data-mode="joystick">Joystick</button>
      </div>

      <div id="tiltMode" class="mode active">
        <div class="tilt-stage">
          <div class="phone-wrap">
            <div class="halo"></div>
            <div id="phone" class="phone"><div class="cross"></div></div>
            <div id="arrow" class="arrow">•</div>
          </div>
        </div>

        <div class="readout">
          <div class="metric"><small>Movimiento</small><strong id="movement">Detenido</strong></div>
          <div id="speedRing" class="speed-ring"><span id="speedText">0%</span></div>
          <div class="metric"><small>Potencia</small><strong id="power">0</strong></div>
        </div>

        <button id="enableTilt" class="primary">Activar inclinación</button>
        <div id="tiltHint" class="hint">Sostén el celular en una posición cómoda y pulsa el botón para calibrarlo.</div>
      </div>

      <div id="joystickMode" class="mode">
        <div class="joystick-stage">
          <div id="joystick" class="joystick">
            <span class="dir up">▲</span><span class="dir down">▼</span>
            <span class="dir left">◀</span><span class="dir right">▶</span>
            <div id="stick" class="stick">✦</div>
          </div>
        </div>
        <div class="readout">
          <div class="metric"><small>Movimiento</small><strong id="movementJoy">Detenido</strong></div>
          <div id="speedRingJoy" class="speed-ring"><span id="speedTextJoy">0%</span></div>
          <div class="metric"><small>Potencia</small><strong id="powerJoy">0</strong></div>
        </div>
        <div class="hint">Arrastra el control. Al soltarlo, el auto se detiene inmediatamente.</div>
      </div>
    </section>

    <div class="footer-status"><span class="mini-dot"></span> Seguridad activa: parada automática por pérdida de señal</div>
  </main>

<script>
(() => {
  'use strict';

  const MAX_PWM = 255;
  const MAX_TILT_DEG = 28;
  const TILT_DEADZONE_DEG = 4;
  const SEND_INTERVAL_MS = 80;

  let desiredLeft = 0;
  let desiredRight = 0;
  let lastSentLeft = null;
  let lastSentRight = null;
  let requestPending = false;
  let activeMode = 'tilt';

  let tiltEnabled = false;
  let calibrated = false;
  let betaZero = 0;
  let gammaZero = 0;

  const $ = id => document.getElementById(id);
  const tabs = [...document.querySelectorAll('.tab')];

  const clamp = (v, min, max) => Math.max(min, Math.min(max, v));
  const deadband = (value, zone) => {
    const a = Math.abs(value);
    if (a <= zone) return 0;
    return Math.sign(value) * ((a - zone) / (1 - zone));
  };

  function setNetwork(ok) {
    $('netDot').classList.toggle('off', !ok);
    $('netText').textContent = ok ? 'Conectado' : 'Sin señal';
  }

  function movementLabel(throttle, turn) {
    if (Math.abs(throttle) < 0.02 && Math.abs(turn) < 0.02) return 'Detenido';
    if (Math.abs(throttle) >= Math.abs(turn)) return throttle > 0 ? 'Adelante' : 'Atrás';
    return turn > 0 ? 'Derecha' : 'Izquierda';
  }

  function updateReadout(throttle, turn, source) {
    const intensity = Math.round(Math.max(Math.abs(throttle), Math.abs(turn)) * 100);
    const power = Math.round(intensity * MAX_PWM / 100);
    const label = movementLabel(throttle, turn);
    const suffix = source === 'joy' ? 'Joy' : '';
    $('movement' + suffix).textContent = label;
    $('power' + suffix).textContent = power;
    $('speedText' + suffix).textContent = intensity + '%';
    $('speedRing' + suffix).style.setProperty('--speed', intensity + '%');

    if (source !== 'joy') {
      const arrow = $('arrow');
      const symbols = {Adelante:'↑',Atrás:'↓',Derecha:'→',Izquierda:'←',Detenido:'•'};
      arrow.textContent = symbols[label];
      arrow.style.opacity = label === 'Detenido' ? '.55' : '1';
    }
  }

  function driveFromAxes(throttle, turn, source) {
    throttle = clamp(throttle, -1, 1);
    turn = clamp(turn, -1, 1);

    // Sin diagonales: solo se conserva el eje dominante.
    if (Math.abs(throttle) >= Math.abs(turn)) turn = 0;
    else throttle = 0;

    let left = 0;
    let right = 0;

    if (throttle !== 0) {
      left = throttle;
      right = throttle;
    } else if (turn !== 0) {
      // Giro tipo tanque: ruedas en sentidos opuestos.
      left = turn;
      right = -turn;
    }

    desiredLeft = Math.round(left * MAX_PWM);
    desiredRight = Math.round(right * MAX_PWM);
    updateReadout(throttle, turn, source);
  }

  function stopNow() {
    desiredLeft = 0;
    desiredRight = 0;
    updateReadout(0, 0, activeMode === 'joystick' ? 'joy' : 'tilt');
    fetch('/stop', {cache:'no-store', keepalive:true}).catch(() => {});
  }

  async function transmit() {
    const changed = desiredLeft !== lastSentLeft || desiredRight !== lastSentRight;
    const moving = desiredLeft !== 0 || desiredRight !== 0;
    if ((!changed && !moving) || requestPending) return;

    requestPending = true;
    try {
      const response = await fetch(`/cmd?l=${desiredLeft}&r=${desiredRight}&t=${Date.now()}`, {cache:'no-store'});
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
  setInterval(transmit, SEND_INTERVAL_MS);

  function calibrate(beta, gamma) {
    betaZero = beta;
    gammaZero = gamma;
    calibrated = true;
    $('enableTilt').textContent = 'Recentrar inclinación';
    $('tiltHint').textContent = 'Inclina hacia adelante o atrás. Inclina a los lados para girar sobre su eje.';
    stopNow();
  }

  function handleOrientation(event) {
    if (!tiltEnabled || activeMode !== 'tilt') return;
    if (event.beta == null || event.gamma == null) return;

    if (!calibrated) {
      calibrate(event.beta, event.gamma);
      return;
    }

    const betaDelta = betaZero - event.beta;
    const gammaDelta = event.gamma - gammaZero;

    let throttle = clamp(betaDelta / MAX_TILT_DEG, -1, 1);
    let turn = clamp(gammaDelta / MAX_TILT_DEG, -1, 1);

    const zone = TILT_DEADZONE_DEG / MAX_TILT_DEG;
    throttle = deadband(throttle, zone);
    turn = deadband(turn, zone);

    $('phone').style.transform = `rotateX(${clamp(-betaDelta, -22, 22)}deg) rotateY(${clamp(gammaDelta, -22, 22)}deg)`;
    driveFromAxes(throttle, turn, 'tilt');
  }

  async function enableTilt() {
    if (!('DeviceOrientationEvent' in window)) {
      $('tiltHint').textContent = 'Este navegador no ofrece sensores de orientación. Usa el joystick.';
      return;
    }

    try {
      if (typeof DeviceOrientationEvent.requestPermission === 'function') {
        const permission = await DeviceOrientationEvent.requestPermission();
        if (permission !== 'granted') throw new Error('Permiso denegado');
      }

      if (!tiltEnabled) {
        window.addEventListener('deviceorientation', handleOrientation, true);
        tiltEnabled = true;
      }
      calibrated = false;
      $('enableTilt').textContent = 'Mantén el celular quieto…';
      $('tiltHint').textContent = 'Calibrando la posición central.';
    } catch (error) {
      $('tiltHint').textContent = 'El navegador bloqueó el sensor en esta página HTTP. Abre 192.168.4.1 en Chrome o usa el joystick.';
      setMode('joystick');
    }
  }

  $('enableTilt').addEventListener('click', enableTilt);

  function setMode(mode) {
    activeMode = mode;
    tabs.forEach(t => t.classList.toggle('active', t.dataset.mode === mode));
    $('tiltMode').classList.toggle('active', mode === 'tilt');
    $('joystickMode').classList.toggle('active', mode === 'joystick');
    stopNow();
    resetJoystick();
  }
  tabs.forEach(tab => tab.addEventListener('click', () => setMode(tab.dataset.mode)));

  const joystick = $('joystick');
  const stick = $('stick');
  let joyPointer = null;

  function updateJoystick(event) {
    const rect = joystick.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    const maxRadius = rect.width * 0.31;

    let dx = event.clientX - centerX;
    let dy = event.clientY - centerY;
    const distance = Math.hypot(dx, dy);
    if (distance > maxRadius) {
      dx = dx / distance * maxRadius;
      dy = dy / distance * maxRadius;
    }

    let x = dx / maxRadius;
    let y = -dy / maxRadius;

    // Un solo eje: vertical o horizontal.
    if (Math.abs(y) >= Math.abs(x)) x = 0;
    else y = 0;

    const visualX = x * maxRadius;
    const visualY = -y * maxRadius;
    stick.style.transform = `translate(${visualX}px,${visualY}px)`;
    driveFromAxes(y, x, 'joy');
  }

  function resetJoystick() {
    joyPointer = null;
    stick.classList.remove('dragging');
    stick.style.transform = 'translate(0,0)';
    if (activeMode === 'joystick') driveFromAxes(0, 0, 'joy');
  }

  joystick.addEventListener('pointerdown', event => {
    joyPointer = event.pointerId;
    joystick.setPointerCapture(event.pointerId);
    stick.classList.add('dragging');
    updateJoystick(event);
  });
  joystick.addEventListener('pointermove', event => {
    if (event.pointerId === joyPointer) updateJoystick(event);
  });
  ['pointerup','pointercancel','lostpointercapture'].forEach(type => {
    joystick.addEventListener(type, event => {
      if (joyPointer === null || event.pointerId === joyPointer) {
        resetJoystick();
        stopNow();
      }
    });
  });

  document.addEventListener('visibilitychange', () => {
    if (document.hidden) stopNow();
  });
  window.addEventListener('pagehide', stopNow);
  window.addEventListener('blur', stopNow);
  window.addEventListener('orientationchange', () => {
    calibrated = false;
    stopNow();
    if (tiltEnabled) $('tiltHint').textContent = 'La pantalla giró. Pulsa “Recentrar inclinación”.';
  });

  // Evita menús, zoom y desplazamientos accidentales durante el manejo.
  document.addEventListener('contextmenu', event => event.preventDefault());
  document.addEventListener('touchmove', event => event.preventDefault(), {passive:false});
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

  // La dirección se establece antes de aplicar PWM.
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

  // Una orden cero siempre frena inmediatamente.
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
  // No se inicia Serial porque GPIO1/TX y GPIO3/RX controlan el motor derecho.
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

  // Portal cautivo: cualquier dominio se resuelve hacia el autito.
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.on("/stop", HTTP_GET, handleStop);
  server.on("/status", HTTP_GET, handleStatus);

  // Rutas comunes que consultan Android, iOS y Windows al detectar un portal.
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
