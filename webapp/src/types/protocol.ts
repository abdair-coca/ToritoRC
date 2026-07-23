export interface ControlCommand {
  leftMotor: number
  rightMotor: number
  gear: number
  frontLights: boolean
  leftBlinker: boolean
  rightBlinker: boolean
  brake: boolean
  horn: boolean
}

export interface Telemetry {
  battery: number
  speed: number
  gear: number
  rpm: number
  frontLights: boolean
  leftBlinker: boolean
  rightBlinker: boolean
  brake: boolean
}

export const GEAR_NAMES = ['N', '1', '2', '3', '4', '5', 'R']
export const GEAR_NEUTRAL = 0
export const GEAR_REVERSE = 6
export const GEAR_MAX = 5
