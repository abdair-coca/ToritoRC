# ToritoRC 🚘⚡

Vehículo a radiocontrol (RC) controlado mediante **WebSockets** desde una aplicación web interactiva (**React + Vite + TypeScript**) conectada a un microcontrolador **ESP8266 (NodeMCU / Wemos D1 Mini)** en modo Access Point.

---

## 📌 Características
- 📡 **Conexión Inalámbrica Local**: Punto de acceso WiFi directo (`ToritoRC`) y comunicación WebSocket a 10 Hz.
- 🕹️ **Control Web Táctil / Teclado**: Joystick de aceleración y dirección con panel de luces, bocina y freno.
- ⚙️ **Transmisión Secuencial**: Simulación de caja de cambios de 6 marchas ($N, 1, 2, 3, 4, 5, R$) con cálculo dinámico de revoluciones (RPM).
- 🔊 **Audio Sintetizado**: Efectos de sonido de motor, intermitentes y bocina mediante Web Audio API en el navegador y salida PWM en firmware.
- 📊 **Telemetría en Tiempo Real**: Medición de voltaje de batería, velocidad estimada, RPM y marcha actual.

---

## 🛠️ Estructura del Proyecto

- `docs/`: Documentación de protocolo WebSocket y roadmap de fases del proyecto.
- `hardware/`: Mapeo de pines para Wemos D1 Mini y Lista de Materiales (BOM).
- `firmware/`: Código C++ para PlatformIO (ESP8266, L298N, luces, audio y servidor WS).
- `webapp/`: Interfaz de usuario construida con React 19, Tailwind CSS v4, Three.js y Vite.

---

## 🚀 Guía Rápida

### 1. Cargar Firmware (ESP8266)
```bash
cd firmware
pio run -t upload
```

### 2. Iniciar WebApp
```bash
cd webapp
npm install
npm run dev
```

### 3. Conectar y Jugar
1. Conecta tu dispositivo al WiFi **SSID: ToritoRC** (contraseña: `toritopass`).
2. Abre la URL local de la WebApp en tu navegador (`http://localhost:5173`).
