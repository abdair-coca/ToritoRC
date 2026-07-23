# ToritoRC — Fases del Proyecto

## FASE 1 — Base Hardware + Protocolo ✅
- [x] 1.1 `hardware/pinout.md` — Mapeo final pines D1 Mini
- [x] 1.2 `docs/protocol.md` — Mensajes JSON WebSocket
- [x] 1.3 `firmware/include/pins.h` — Constantes pines
- [x] 1.4 `firmware/src/network/` — Modo AP + WebSocket server
- [x] 1.5 `firmware/src/protocol/` — Parseo JSON comandos
- [ ] 1.6 Test: ESP8266 responde a WebSocket

## FASE 2 — Motores + Transmisión ✅
- [x] 2.1 `firmware/src/motor/` — PWM motores + direccion L298N
- [x] 2.2 `firmware/src/transmission/` — Logica cambios secuencial
- [x] 2.3 Integrar motor + transmision en `main.cpp`
- [ ] 2.4 Lectura encoders (velocidad real)
- [ ] 2.5 Test: torito avanza, cambia marchas, frena

## FASE 3 — Luces ✅
- [x] 3.1 `firmware/src/lights/` — Faros, intermitentes, freno
- [x] 3.2 Intermitentes sincronizados con direccion
- [ ] 3.3 Freno automatico al desacelerar brusco
- [ ] 3.4 Test: todas las luces funcionan

## FASE 4 — Sonido ✅
- [x] 4.1 `firmware/src/audio/` — Sintesis motor por PWM
- [x] 4.2 Sonido cambia segun RPM y marcha
- [x] 4.3 Clic intermitente + bocina (Web Audio API)
- [ ] 4.4 Test: audio sincronizado

## FASE 5 — Frontend Web ✅
- [x] 5.1 Init Vite + React + Tailwind + Three.js
- [x] 5.2 `useWebSocket` hook + conexion al ESP8266
- [x] 5.3 `Controls.tsx` — Joystick touch + freno
- [x] 5.4 `TransmissionPanel.tsx` — Marchas secuencial
- [x] 5.5 `LightsPanel.tsx` — Faros, intermitentes, bocina
- [x] 5.6 `Dashboard.tsx` — Velocimetro, RPM, bateria
- [x] 5.7 `Torito3D.tsx` — Modelo 3D (SVG, pendiente Three.js)
- [x] 5.8 `useAudio.ts` — Sonidos sincronizados via Web Audio
- [ ] 5.9 Integracion final web -> firmware

## FASE 6 — Integracion + Testing
- [ ] 6.1 Probar web + firmware juntos
- [ ] 6.2 Ajustar latencia, calibracion motores
- [ ] 6.3 Wiring final en protoboard
- [ ] 6.4 Documentacion final (README, fotos)
