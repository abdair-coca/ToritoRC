import { useRef, useState, useCallback } from 'react'

interface Props {
  onThrottle: (value: number) => void
}

export function ThrottleLever({ onThrottle }: Props) {
  const trackRef = useRef<HTMLDivElement>(null)
  const [pos, setPos] = useState(0)
  const activeTouch = useRef<number | null>(null)
  const baseY = useRef(0)
  const baseVal = useRef(0)

  const updateThrottle = useCallback((y: number) => {
    const track = trackRef.current
    if (!track) return
    const rect = track.getBoundingClientRect()
    const dy = (baseY.current - y) / rect.height
    const val = Math.max(0, Math.min(1, baseVal.current + dy))
    setPos(val)
    onThrottle(Math.round(val * 255))
  }, [onThrottle])

  const handleTouchStart = useCallback((e: React.TouchEvent) => {
    e.preventDefault()
    const touch = e.changedTouches[0]
    activeTouch.current = touch.identifier
    baseY.current = touch.clientY
    baseVal.current = pos
  }, [pos])

  const handleTouchMove = useCallback((e: React.TouchEvent) => {
    e.preventDefault()
    const touch = Array.from(e.changedTouches).find(t => t.identifier === activeTouch.current)
    if (!touch) return
    updateThrottle(touch.clientY)
  }, [updateThrottle])

  const handleTouchEnd = useCallback(() => {
    activeTouch.current = null
    setPos(0)
    onThrottle(0)
  }, [onThrottle])

  const pct = pos * 100

  return (
    <div className="flex flex-col items-center gap-1">
      <div className="text-[8px] text-zinc-500 font-mono tracking-widest">| THROTTLE |</div>

      {/* Track */}
      <div
        ref={trackRef}
        onTouchStart={handleTouchStart}
        onTouchMove={handleTouchMove}
        onTouchEnd={handleTouchEnd}
        className="relative w-10 h-28 rounded-full touch-none select-none cursor-pointer"
        style={{
          background: 'linear-gradient(to top, #1A1A1A, #2A2A2A)',
          border: '1px solid #444',
          boxShadow: 'inset 0 2px 6px rgba(0,0,0,0.6), inset 0 -1px 0 rgba(255,255,255,0.05)',
        }}
      >
        {/* Fill gradient */}
        <div className="absolute bottom-0 left-0 right-0 rounded-full transition-all duration-75"
          style={{
            height: `${pct}%`,
            background: `linear-gradient(to top, #F5A623, #FF6B35)`,
            opacity: pos > 0 ? 0.6 : 0,
          }} />

        {/* Notches */}
        <div className="absolute inset-x-2 top-[15%] h-px bg-zinc-700/50" />
        <div className="absolute inset-x-2 top-[30%] h-px bg-zinc-700/50" />
        <div className="absolute inset-x-2 top-[50%] h-px bg-amber-700/30" />
        <div className="absolute inset-x-2 top-[70%] h-px bg-zinc-700/50" />
        <div className="absolute inset-x-2 top-[85%] h-px bg-zinc-700/50" />

        {/* Knob */}
        <div
          className="absolute left-1/2 -translate-x-1/2 w-9 h-9 rounded-full transition-all duration-75"
          style={{
            top: `calc(${100 - pct}% - 18px)`,
            background: 'radial-gradient(circle at 35% 35%, #666, #2A2A2A 60%, #1A1A1A)',
            border: '1px solid #555',
            boxShadow: '0 2px 8px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,255,255,0.15)',
          }}
        >
          <div className="absolute inset-1 rounded-full"
            style={{
              background: 'radial-gradient(circle at 35% 30%, rgba(255,255,255,0.1), transparent 60%)',
            }} />
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="w-1.5 h-1.5 rounded-full bg-zinc-500" />
          </div>
        </div>
      </div>

      {/* Label */}
      <div className="text-[7px] text-zinc-600 font-mono">SPRING-RETURN</div>
    </div>
  )
}
