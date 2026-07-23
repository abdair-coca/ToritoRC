import { useState, useEffect, useRef, useCallback } from 'react'

type ControlState = {
  throttle: number
  steerDir: 'left' | 'right' | null
  gear: number
  frontLights: boolean
  leftBlinker: boolean
  rightBlinker: boolean
  brake: boolean
  horn: boolean
}

export type Telemetry = {
  battery: number
  speed: number
  gear: number
  rpm: number
  frontLights: boolean
  leftBlinker: boolean
  rightBlinker: boolean
  brake: boolean
  horn: boolean
  leftMotor: number
  rightMotor: number
}

const DEFAULT_CTRL: ControlState = {
  throttle: 0, steerDir: null, gear: 0,
  frontLights: false, leftBlinker: false, rightBlinker: false,
  brake: false, horn: false,
}

export function useSimulation() {
  const [telemetry, setTelemetry] = useState<Telemetry>({
    battery: 7.4, speed: 0, gear: 0, rpm: 0,
    frontLights: false, leftBlinker: false, rightBlinker: false, brake: false, horn: false,
    leftMotor: 0, rightMotor: 0,
  })
  const [connected] = useState(false)
  const ctrlRef = useRef<ControlState>({ ...DEFAULT_CTRL })
  const simRef = useRef({ speed: 0, rpm: 0 })

  const getCtrl = useCallback(() => ({ ...ctrlRef.current }), [])

  const setCtrl = useCallback((partial: Partial<ControlState>) => {
    const ctrl = ctrlRef.current
    Object.assign(ctrl, partial)

    setTelemetry(prev => ({
      ...prev,
      gear: ctrl.gear,
      frontLights: ctrl.frontLights,
      leftBlinker: ctrl.leftBlinker,
      rightBlinker: ctrl.rightBlinker,
      brake: ctrl.brake,
      horn: ctrl.horn,
    }))
  }, [])

  useEffect(() => {
    let frame: number
    const step = () => {
      const ctrl = ctrlRef.current
      const sim = simRef.current
      const t = ctrl.throttle / 255

      // Calculate motor outputs
      let left = t
      let right = t
      if (ctrl.steerDir === 'left') { left *= 0.3; right *= 1.0 }
      if (ctrl.steerDir === 'right') { left *= 1.0; right *= 0.3 }

      // Reverse
      if (ctrl.gear === 6) { left = -left; right = -right }

      const brakeMultiplier = ctrl.brake ? 0.3 : 1.0
      const speedFactor = ctrl.gear === 0 ? 0 : ctrl.gear === 6 ? 0.15 : ctrl.gear / 5
      const targetSpeed = t * speedFactor * 60 * brakeMultiplier
      sim.speed += (targetSpeed - sim.speed) * (ctrl.brake ? 0.15 : 0.08)
      if (Math.abs(sim.speed) < 0.05) sim.speed = 0

      const targetRpm = t * (ctrl.gear === 0 ? 0 : 2000 + (ctrl.gear === 6 ? 0 : ctrl.gear * 1000)) * brakeMultiplier
      sim.rpm += (targetRpm - sim.rpm) * 0.15
      if (Math.abs(sim.rpm) < 10) sim.rpm = 0

      setTelemetry(prev => ({
        ...prev,
        speed: sim.speed,
        rpm: Math.round(sim.rpm),
        leftMotor: Math.round(left * 255),
        rightMotor: Math.round(right * 255),
      }))

      frame = requestAnimationFrame(step)
    }
    frame = requestAnimationFrame(step)
    return () => cancelAnimationFrame(frame)
  }, [])

  return { telemetry, setCtrl, getCtrl, connected }
}
