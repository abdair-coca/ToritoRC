import { useEffect, useRef, useState } from 'react'
import { wsService } from '../services/websocket'
import type { Telemetry } from '../types/protocol'

export function useWebSocket() {
  const [connected, setConnected] = useState(false)
  const [telemetry, setTelemetry] = useState<Telemetry>({
    battery: 0, speed: 0, gear: 0, rpm: 0,
    frontLights: false, leftBlinker: false, rightBlinker: false, brake: false,
  })
  const connectedRef = useRef(connected)

  useEffect(() => {
    wsService.onTelemetry((data) => {
      setTelemetry(data)
    })

    const check = setInterval(() => {
      const c = wsService.connected
      if (c !== connectedRef.current) {
        connectedRef.current = c
        setConnected(c)
      }
    }, 200)

    wsService.connect()

    return () => { clearInterval(check); wsService.disconnect() }
  }, [])

  return { connected, telemetry }
}
