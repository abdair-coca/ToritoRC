import { useCallback } from 'react'

interface Props {
  onSteer: (dir: 'left' | 'right' | null) => void
}

export function SteeringButtons({ onSteer }: Props) {
  const handleLeftStart = useCallback(() => onSteer('left'), [onSteer])
  const handleRightStart = useCallback(() => onSteer('right'), [onSteer])
  const handleEnd = useCallback(() => onSteer(null), [onSteer])

  return (
    <div className="flex flex-col items-center gap-1">
      <div className="text-[8px] text-zinc-500 font-mono tracking-widest">| STEER |</div>
      <div className="flex gap-2">
        {/* Left button */}
        <button
          onTouchStart={handleLeftStart}
          onTouchEnd={handleEnd}
          onTouchCancel={handleEnd}
          className="w-14 h-20 rounded-xl touch-none select-none active:scale-95 transition-transform duration-75"
          style={{
            background: 'linear-gradient(180deg, #2A2A2A, #1A1A1A 60%, #0D0D0D)',
            border: '1px solid #444',
            boxShadow: '0 2px 8px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.06)',
          }}
        >
          <div className="flex flex-col items-center gap-1">
            <div className="text-amber-500 text-lg font-bold" style={{ fontFamily: 'Oswald,sans-serif' }}>◄</div>
            <div className="text-[7px] text-zinc-500 font-mono">IZQ</div>
          </div>
        </button>

        {/* Right button */}
        <button
          onTouchStart={handleRightStart}
          onTouchEnd={handleEnd}
          onTouchCancel={handleEnd}
          className="w-14 h-20 rounded-xl touch-none select-none active:scale-95 transition-transform duration-75"
          style={{
            background: 'linear-gradient(180deg, #2A2A2A, #1A1A1A 60%, #0D0D0D)',
            border: '1px solid #444',
            boxShadow: '0 2px 8px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.06)',
          }}
        >
          <div className="flex flex-col items-center gap-1">
            <div className="text-amber-500 text-lg font-bold" style={{ fontFamily: 'Oswald,sans-serif' }}>►</div>
            <div className="text-[7px] text-zinc-500 font-mono">DER</div>
          </div>
        </button>
      </div>
    </div>
  )
}
