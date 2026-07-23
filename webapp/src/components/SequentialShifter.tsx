import { useRef, useCallback } from 'react'
import { GEAR_NAMES, GEAR_MAX } from '../types/protocol'

interface Props {
  currentGear: number
  onChange: (gear: number) => void
}

export function SequentialShifter({ currentGear, onChange }: Props) {
  const knobRef = useRef<HTMLDivElement>(null)
  const activeTouch = useRef<number | null>(null)
  const startY = useRef(0)
  const shifted = useRef(false)

  const doShift = useCallback((dir: 'up' | 'down') => {
    if (shifted.current) return
    shifted.current = true

    let gear = currentGear
    if (dir === 'up') {
      if (currentGear === 0) gear = 1
      else if (currentGear === 6) gear = 0
      else gear = Math.min(currentGear + 1, GEAR_MAX)
    } else {
      if (currentGear === 1) gear = 0
      else if (currentGear === 0) gear = 6
      else gear = Math.max(currentGear - 1, 0)
    }

    if (gear !== currentGear) {
      onChange(gear)
      if (navigator.vibrate) navigator.vibrate(15)
    }

    if (knobRef.current) {
      knobRef.current.style.transform = dir === 'up'
        ? 'translateY(-8px) rotateX(5deg)'
        : 'translateY(8px) rotateX(-5deg)'
    }

    setTimeout(() => {
      if (knobRef.current) knobRef.current.style.transform = 'translateY(0) rotateX(0)'
      shifted.current = false
    }, 150)
  }, [currentGear, onChange])

  const handleTouchStart = useCallback((e: React.TouchEvent) => {
    const touch = e.changedTouches[0]
    activeTouch.current = touch.identifier
    startY.current = touch.clientY
  }, [])

  const handleTouchMove = useCallback((e: React.TouchEvent) => {
    const touch = Array.from(e.changedTouches).find(t => t.identifier === activeTouch.current)
    if (!touch) return
    const dy = touch.clientY - startY.current
    if (Math.abs(dy) > 30 && !shifted.current) {
      doShift(dy < 0 ? 'up' : 'down')
    }
  }, [doShift])

  const handleTouchEnd = useCallback(() => {
    activeTouch.current = null
  }, [])

  return (
    <div className="flex flex-col items-center gap-1">
      <div className="text-[8px] text-zinc-500 font-mono tracking-widest">| GEAR |</div>
      <div className="relative flex flex-col items-center">
        <div className="w-1 h-20 bg-gradient-to-b from-zinc-600 via-zinc-500 to-zinc-600 rounded-full"
          style={{ boxShadow: 'inset 0 1px 2px rgba(0,0,0,0.5)' }} />
        <div className="absolute top-0 left-1/2 -translate-x-1/2 w-4 h-[2px] bg-zinc-400 rounded-full" style={{ top: '2px' }} />
        <div className="absolute top-1/2 left-1/2 -translate-x-1/2 w-5 h-[2px] bg-amber-600 rounded-full" />
        <div className="absolute bottom-0 left-1/2 -translate-x-1/2 w-4 h-[2px] bg-zinc-400 rounded-full" style={{ bottom: '2px' }} />

        <div
          ref={knobRef}
          onTouchStart={handleTouchStart}
          onTouchMove={handleTouchMove}
          onTouchEnd={handleTouchEnd}
          className="absolute top-1/2 -translate-x-1/2 -translate-y-1/2 w-10 h-10 rounded-lg cursor-pointer touch-none select-none z-10 transition-transform duration-100"
          style={{
            background: 'linear-gradient(180deg, #3B2A1E 0%, #1A1A1A 100%)',
            border: '1px solid #5A4A3A',
            boxShadow: '0 2px 6px rgba(0,0,0,0.6), inset 0 1px 0 rgba(255,255,255,0.1)',
          }}
        >
          <div className="w-full h-full rounded-lg flex items-center justify-center"
            style={{
              background: 'linear-gradient(135deg, rgba(255,255,255,0.05) 0%, transparent 50%)',
            }}>
            <div className="w-7 h-7 rounded border border-amber-700/30 flex items-center justify-center">
              <div className="text-amber-500 font-bold text-xs font-mono">{GEAR_NAMES[currentGear]}</div>
            </div>
          </div>
        </div>
      </div>

      <div className="flex flex-col items-center text-[8px] text-zinc-500 font-mono leading-tight">
        <span className="text-zinc-400">▲ UP</span>
        <span className="text-zinc-600">|</span>
        <span className="text-zinc-400">▼ DOWN</span>
      </div>
    </div>
  )
}
