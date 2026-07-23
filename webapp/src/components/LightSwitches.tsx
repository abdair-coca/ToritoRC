import { useCallback } from 'react'

interface Props {
  data: any
  onChange: (field: string, value: boolean) => void
}

function SwitchBtn({ label, active, color, onToggle }: {
  label: string; active: boolean; color: string; onToggle: () => void
}) {
  return (
    <button
      onTouchStart={onToggle}
      className="relative w-12 h-12 rounded-lg touch-none select-none transition-all duration-100 active:scale-95"
      style={{
        background: active
          ? `linear-gradient(180deg, ${color}dd 0%, ${color}99 100%)`
          : 'linear-gradient(180deg, #2A2A2A 0%, #1A1A1A 100%)',
        border: active ? `1px solid ${color}88` : '1px solid #333',
        boxShadow: active
          ? `inset 0 -2px 4px rgba(0,0,0,0.4), 0 0 8px ${color}44`
          : 'inset 0 -2px 4px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.05)',
      }}
    >
      <div className="absolute top-1 left-1/2 -translate-x-1/2 w-1.5 h-1.5 rounded-full transition-all duration-200"
        style={{
          background: active ? color : '#333',
          boxShadow: active ? `0 0 4px ${color}` : 'none',
        }} />
      <span className="text-[8px] font-bold leading-tight text-center px-0.5"
        style={{ color: active ? '#fff' : '#666' }}>
        {label}
      </span>
      {active && (
        <div className="absolute bottom-1 left-1/2 -translate-x-1/2 w-3 h-[2px] rounded-full"
          style={{ background: color, boxShadow: `0 0 3px ${color}` }} />
      )}
    </button>
  )
}

export function LightSwitches({ data, onChange }: Props) {
  const toggle = useCallback((field: string) => {
    onChange(field, !data[field])
  }, [data, onChange])

  return (
    <div className="flex flex-wrap items-center justify-center gap-1.5 max-w-[160px]">
      <SwitchBtn label="FAROS" active={data.frontLights} color="#FFD700" onToggle={() => toggle('frontLights')} />
      <SwitchBtn label="◄" active={data.leftBlinker} color="#FFB020" onToggle={() => toggle('leftBlinker')} />
      <SwitchBtn label="►" active={data.rightBlinker} color="#FFB020" onToggle={() => toggle('rightBlinker')} />
      <SwitchBtn label="⚠" active={data.leftBlinker && data.rightBlinker} color="#E63946" onToggle={() => {
        const on = !(data.leftBlinker && data.rightBlinker)
        onChange('leftBlinker', on)
        onChange('rightBlinker', on)
      }} />
      <SwitchBtn label="BOCINA" active={data.horn} color="#E63946" onToggle={() => toggle('horn')} />
    </div>
  )
}
