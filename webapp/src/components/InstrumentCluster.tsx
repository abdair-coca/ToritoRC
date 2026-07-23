import { AnalogGauge } from './AnalogGauge'
import type { Telemetry } from '../types/protocol'

interface Props { data: Telemetry }

function Indicator({ label, active, color, blink }: {
  label: string; active: boolean; color: string; blink?: boolean
}) {
  return (
    <div className={`flex items-center gap-1 px-1.5 py-0.5 rounded text-[7px] font-bold font-mono tracking-wider
      ${active ? 'bg-black/60 border' : 'bg-black/20 border border-zinc-800/30 opacity-30'}`}
      style={{
        borderColor: active ? `${color}66` : 'transparent',
        boxShadow: active ? `inset 0 0 6px ${color}22, 0 0 4px ${color}11` : 'none',
      }}>
      <div className={`w-2 h-2 rounded-full ${blink && active ? 'animate-pulse' : ''}`}
        style={{
          background: active ? color : '#333',
          boxShadow: active ? `0 0 4px ${color}` : 'none',
        }} />
      <span style={{ color: active ? color : '#444' }}>{label}</span>
    </div>
  )
}

export function InstrumentCluster({ data }: Props) {
  const batteryPct = Math.round(((data.battery - 6) / (8.4 - 6)) * 100)

  return (
    <div className="flex flex-col items-center gap-1.5 px-2 pt-2 pb-2">
      {/* Indicator lights row */}
      <div className="flex items-center gap-1.5 flex-wrap justify-center">
        <Indicator label="◄" active={data.leftBlinker} color="#4CAF50" blink />
        <Indicator label="N" active={data.gear === 0} color="#4CAF50" />
        <Indicator label="R" active={data.gear === 6} color="#E63946" />
        <Indicator label="►" active={data.rightBlinker} color="#4CAF50" blink />
        <Indicator label="BRAKE" active={data.brake} color="#E63946" />
        <Indicator label="FAROS" active={data.frontLights} color="#FFD700" />
      </div>

      {/* Gauges row */}
      <div className="flex items-center justify-center gap-2 md:gap-4">
        {/* Tachometer */}
        <AnalogGauge
          value={data.rpm}
          max={8000}
          unit="RPM ×1000"
          label="tacho"
          size={130}
          redline={6500}
          warnStart={5500}
          color="#FF6B35"
        />

        {/* Digital display center */}
        <div className="flex flex-col items-center gap-1.5 px-2 py-1 w-16 md:w-20">
          {/* Gear display */}
          <div className="w-full bg-black/80 border border-amber-600/60 rounded-lg pt-1.5 pb-1 text-center"
            style={{ boxShadow: 'inset 0 0 12px rgba(255,107,0,0.12), 0 0 6px rgba(255,107,0,0.08)' }}>
            <div className="text-amber-400 font-bold tabular-nums leading-none drop-shadow-[0_0_6px_rgba(255,175,0,0.3)]"
              style={{ fontFamily: 'Oswald,sans-serif', fontSize: 30 }}>
              {data.gear === 0 ? 'N' : data.gear === 6 ? 'R' : data.gear}
            </div>
            <div className="text-[7px] text-amber-700/80 font-mono tracking-widest">GEAR</div>
          </div>

          {/* Battery */}
          <div className="flex items-center gap-1.5 text-[10px] font-mono bg-black/40 rounded px-1.5 py-0.5 border border-zinc-700/50 w-full justify-center">
            <div className="w-6 h-3 border border-zinc-400 rounded-sm relative flex items-center bg-black/60">
              <div className={`h-2 rounded-sm mx-[1px] transition-all ${batteryPct < 20 ? 'bg-red-500' : 'bg-green-400'}`}
                style={{ width: `${Math.max(batteryPct, 5)}%` }} />
            </div>
            <span className={batteryPct < 20 ? 'text-red-400 font-semibold' : 'text-zinc-300'}>{batteryPct}%</span>
          </div>

          {/* Speed digital */}
          <div className="text-center bg-black/40 rounded px-2 py-0.5 border border-zinc-700/30 w-full">
            <div className="text-white font-bold tabular-nums leading-none drop-shadow-[0_0_4px_rgba(255,255,255,0.1)]"
              style={{ fontFamily: 'Oswald,sans-serif', fontSize: 18 }}>
              {(data.speed * 3.6).toFixed(0)}
            </div>
            <div className="text-[7px] text-zinc-500 font-mono tracking-wider">KM/H</div>
          </div>
        </div>

        {/* Speedometer */}
        <AnalogGauge
          value={data.speed * 3.6}
          max={60}
          unit="km/h"
          label="speedo"
          size={150}
          warnStart={30}
          redline={45}
          color="#FF6B35"
        />
      </div>
    </div>
  )
}
