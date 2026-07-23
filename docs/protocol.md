# Protocolo WebSocket — ToritoRC

## Conexion
- ESP8266 crea AP: SSID `ToritoRC`, pass `toritopass`
- WebSocket endpoint: `ws://192.168.4.1:81`
- Formato: JSON

## Comandos (Web -> ESP8266)

```json
{
  "leftMotor": 200,
  "rightMotor": -200,
  "gear": 2,
  "frontLights": true,
  "leftBlinker": false,
  "rightBlinker": false,
  "brake": false,
  "horn": false
}
```

| Campo         | Tipo    | Rango      | Descripcion                     |
|---------------|---------|------------|----------------------------------|
| leftMotor     | int     | -255..255  | Velocidad motor izquierdo        |
| rightMotor    | int     | -255..255  | Velocidad motor derecho          |
| gear          | int     | 0..6       | 0=N, 1-5, 6=R                   |
| frontLights   | bool    |            | Faros delanteros                 |
| leftBlinker   | bool    |            | Intermitente izquierdo           |
| rightBlinker  | bool    |            | Intermitente derecho             |
| brake         | bool    |            | Luz de freno                     |
| horn          | bool    |            | Bocina                           |

## Telemetria (ESP8266 -> Web, 10 Hz)

```json
{
  "battery": 7.2,
  "speed": 0.45,
  "gear": 2,
  "rpm": 3200,
  "frontLights": true,
  "leftBlinker": false,
  "rightBlinker": false,
  "brake": false
}
```

| Campo       | Tipo  | Unidad       | Descripcion                |
|-------------|-------|--------------|----------------------------|
| battery     | float | V            | Voltaje bateria            |
| speed       | float | m/s          | Velocidad estimada         |
| gear        | int   |              | Marcha actual              |
| rpm         | int   |              | RPM motor simulado         |
| frontLights | bool  |              | Estado faros               |
| leftBlinker | bool  |              | Estado intermitente izq    |
| rightBlinker| bool  |              | Estado intermitente der    |
| brake       | bool  |              | Estado freno               |
