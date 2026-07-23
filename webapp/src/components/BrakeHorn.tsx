interface Props {
  brake: boolean
  horn: boolean
  onBrake: () => void
  onHorn: () => void
}

export function BrakeHorn({ brake, horn, onBrake, onHorn }: Props) {
  return (
    <div className="flex flex-col items-center gap-1">
      <div className="text-[8px] text-zinc-500 font-mono tracking-widest">| ACTIONS |</div>
      <div className="flex gap-2">
        {/* Brake */}
        <button
          onTouchStart={onBrake}
          className="w-12 h-12 rounded-xl touch-none select-none active:scale-95 transition-transform duration-75"
          style={{
            background: brake
              ? 'linear-gradient(180deg, #DC2626, #991B1B 60%, #7F1D1D)'
              : 'linear-gradient(180deg, #2A2A2A, #1A1A1A 60%, #0D0D0D)',
            border: `1px solid ${brake ? '#EF4444' : '#444'}`,
            boxShadow: brake
              ? '0 0 16px rgba(239,68,68,0.3), inset 0 1px 0 rgba(255,255,255,0.06)'
              : '0 2px 8px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.06)',
          }}
        >
          <div className="flex flex-col items-center">
            <div className={`text-sm font-bold ${brake ? 'text-white' : 'text-red-500'}`}
              style={{ fontFamily: 'Oswald,sans-serif' }}>BRAKE</div>
          </div>
        </button>

        {/* Horn */}
        <button
          onTouchStart={onHorn}
          className="w-12 h-12 rounded-xl touch-none select-none active:scale-95 transition-transform duration-75"
          style={{
            background: horn
              ? 'linear-gradient(180deg, #F59E0B, #B45309 60%, #92400E)'
              : 'linear-gradient(180deg, #2A2A2A, #1A1A1A 60%, #0D0D0D)',
            border: `1px solid ${horn ? '#F59E0B' : '#444'}`,
            boxShadow: horn
              ? '0 0 16px rgba(245,158,11,0.3), inset 0 1px 0 rgba(255,255,255,0.06)'
              : '0 2px 8px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.06)',
          }}
        >
          <div className="flex flex-col items-center">
            <div className={`text-sm ${horn ? 'text-white' : 'text-amber-500'}`}
              style={{ fontFamily: 'Oswald,sans-serif' }}>HORN</div>
          </div>
        </button>
      </div>
    </div>
  )
}
