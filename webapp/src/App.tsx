import { useEffect, useCallback } from 'react'
import { useSimulation } from './hooks/useSimulation'
import { useAudio } from './hooks/useAudio'
import { wsService } from './services/websocket'
import { InstrumentCluster } from './components/InstrumentCluster'
import { ThrottleLever } from './components/ThrottleLever'
import { SteeringButtons } from './components/SteeringButtons'
import { SequentialShifter } from './components/SequentialShifter'
import { LightSwitches } from './components/LightSwitches'
import { ConnectionStatus } from './components/ConnectionStatus'
import { BrakeHorn } from './components/BrakeHorn'
import type { ControlCommand } from './types/protocol'

function App() {
  const { telemetry, setCtrl, getCtrl, connected } = useSimulation()
  const { updateEngine, updateBlinker } = useAudio()

  const sendFullState = useCallback(() => {
    const ctrl = getCtrl()
    const cmd: ControlCommand = {
      leftMotor: telemetry.leftMotor,
      rightMotor: telemetry.rightMotor,
      gear: ctrl.gear,
      frontLights: ctrl.frontLights,
      leftBlinker: ctrl.leftBlinker,
      rightBlinker: ctrl.rightBlinker,
      brake: ctrl.brake,
      horn: ctrl.horn,
    }
    wsService.send(cmd)
  }, [getCtrl, telemetry.leftMotor, telemetry.rightMotor])

  useEffect(() => {
    updateEngine({ rpm: telemetry.rpm, gear: telemetry.gear, brake: telemetry.brake, blinker: false })
  }, [telemetry.rpm, telemetry.gear, telemetry.brake, updateEngine])

  useEffect(() => {
    const active = telemetry.leftBlinker || telemetry.rightBlinker
    updateBlinker(active)
  }, [telemetry.leftBlinker, telemetry.rightBlinker, updateBlinker])

  const handleGear = (gear: number) => {
    setCtrl({ gear })
    sendFullState()
  }

  const handleThrottle = (value: number) => {
    setCtrl({ throttle: value })
    setTimeout(sendFullState, 0)
  }

  const handleSteer = (dir: 'left' | 'right' | null) => {
    setCtrl({ steerDir: dir })
  }

  const handleLight = (field: string, value: boolean) => {
    setCtrl({ [field]: value } as any)
    setTimeout(sendFullState, 0)
  }

  const handleBrake = () => {
    const ctrl = getCtrl()
    setCtrl({ brake: !ctrl.brake })
    setTimeout(sendFullState, 0)
  }

  const handleHorn = () => {
    const ctrl = getCtrl()
    setCtrl({ horn: !ctrl.horn })
    setTimeout(sendFullState, 0)
  }

  return (
    <div className="relative w-full h-dvh bg-gradient-to-b from-[#0A0A0A] via-[#111] to-[#0A0A0A] overflow-hidden flex flex-col md:flex-row">
      {!connected && (
        <div className="absolute inset-0 z-50 pointer-events-none"
          style={{ boxShadow: 'inset 0 0 100px rgba(230,57,70,0.12)' }} />
      )}

      {/* LEFT — Instruments + brand */}
      <div className="flex-shrink-0 flex flex-col items-center md:w-1/2 md:h-full md:justify-center pt-2 md:pt-0">
        <ConnectionStatus connected={connected} />
        <div className="bg-[#1A1A1A] border border-zinc-700/60 rounded-2xl mx-2 md:mx-4 mt-1 md:mt-2 shadow-lg shadow-black/40"
          style={{ boxShadow: '0 8px 32px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.03)' }}>
          <InstrumentCluster data={telemetry} />
        </div>
        <div className="text-center select-none mt-2 md:mt-4">
          <div className="text-xl md:text-3xl font-bold tracking-widest"
            style={{
              fontFamily: 'Oswald,sans-serif',
              background: 'linear-gradient(180deg, #F5A623 0%, #FFC72C 50%, #F5A623 100%)',
              WebkitBackgroundClip: 'text',
              WebkitTextFillColor: 'transparent',
              filter: 'drop-shadow(0 2px 4px rgba(245,166,35,0.2))',
            }}>
            TORITO
          </div>
          <div className="text-[9px] md:text-[11px] tracking-[0.3em] text-zinc-600 font-mono mt-0.5">REMOTE CONTROL</div>
          <div className="mt-1 w-12 md:w-20 h-[1px] mx-auto bg-gradient-to-r from-transparent via-amber-600/30 to-transparent" />
        </div>
      </div>

      {/* RIGHT — Controls */}
      <div className="flex-shrink-0 md:w-1/2 md:h-full md:flex md:flex-col md:justify-center pb-2 md:pb-0">
        <div className="flex justify-center gap-0.5 mb-1 md:mb-3 mt-1">
          {[1,2,3,4].map(i => (
            <div key={i} className={`w-[3px] md:w-[4px] rounded-sm ${connected ? 'bg-green-500' : 'bg-zinc-500'}`}
              style={{ height: 4 + i * 3, opacity: connected ? 1 : 0.5 }} />
          ))}
          <span className={`text-[8px] md:text-[10px] font-mono ml-1.5 ${connected ? 'text-green-400' : 'text-amber-500'}`}>
            {connected ? 'CONNECTED' : 'DEMO MODE'}
          </span>
        </div>

        <div className="bg-[#1A1A1A] border border-zinc-700/60 rounded-2xl mx-2 p-3 md:p-5 shadow-lg shadow-black/40"
          style={{ boxShadow: '0 8px 32px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.03)' }}>
          <div className="flex items-center justify-center gap-4 md:gap-6">
            {/* Left: Steering + Throttle */}
            <div className="flex flex-col items-center gap-3">
              <SteeringButtons onSteer={handleSteer} />
              <ThrottleLever onThrottle={handleThrottle} />
              <BrakeHorn
                brake={telemetry.brake}
                horn={telemetry.horn}
                onBrake={handleBrake}
                onHorn={handleHorn}
              />
            </div>

            {/* Divider */}
            <div className="w-px h-40 md:h-48 bg-gradient-to-b from-transparent via-zinc-600/30 to-transparent" />

            {/* Right: Shifter + Lights */}
            <div className="flex flex-col items-center gap-3">
              <SequentialShifter currentGear={telemetry.gear} onChange={handleGear} />
              <LightSwitches data={telemetry} onChange={handleLight} />
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default App
