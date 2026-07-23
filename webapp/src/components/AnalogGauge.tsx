import { useRef, useEffect, useState } from 'react'

interface Props {
  value: number
  min?: number
  max: number
  unit: string
  label: string
  size?: number
  redline?: number
  warnStart?: number
  color?: string
}

export function AnalogGauge({
  value, min = 0, max, unit, label,
  size = 160, redline, warnStart, color = '#FF6B35'
}: Props) {
  const [displayValue, setDisplayValue] = useState(min)
  const animRef = useRef<number>(0)
  const currentRef = useRef(min)

  const cx = size / 2
  const cy = size / 2
  const r = size * 0.38
  const startAngle = 225
  const endAngle = -45
  const range = endAngle - startAngle

  useEffect(() => {
    const target = Math.max(min, Math.min(max, value))
    const start = currentRef.current
    const diff = target - start
    const duration = 200
    const startTime = performance.now()

    const animate = (now: number) => {
      const elapsed = now - startTime
      const t = Math.min(elapsed / duration, 1)
      const overshoot = 1 - Math.pow(1 - t, 3) + 0.05 * Math.sin(t * Math.PI)
      const eased = Math.min(overshoot, 1.02)
      const current = start + diff * eased
      currentRef.current = current
      setDisplayValue(current)
      if (t < 1) animRef.current = requestAnimationFrame(animate)
    }

    cancelAnimationFrame(animRef.current)
    animRef.current = requestAnimationFrame(animate)

    return () => cancelAnimationFrame(animRef.current)
  }, [value, min, max])

  const val = Math.max(min, Math.min(max, displayValue))
  const angle = startAngle + (val - min) / (max - min) * range
  const rad = angle * Math.PI / 180

  const needleLen = r * 0.85
  const nx = cx + needleLen * Math.cos(rad)
  const ny = cy + needleLen * Math.sin(rad)

  const ticks = [] as any[]
  const step = max <= 60 ? 10 : 1000
  for (let i = min; i <= max; i += step) {
    const a = startAngle + (i - min) / (max - min) * range
    const rad_a = a * Math.PI / 180
    const outer = r - 2
    const inner = i % (step * (max <= 60 ? 1 : 2)) === 0 ? r - 10 : r - 6
    const x1 = cx + inner * Math.cos(rad_a)
    const y1 = cy + inner * Math.sin(rad_a)
    const x2 = cx + outer * Math.cos(rad_a)
    const y2 = cy + outer * Math.sin(rad_a)
    ticks.push(<line key={i} x1={x1} y1={y1} x2={x2} y2={y2} stroke={redline && i >= redline ? '#E63946' : '#D0D0D0'} strokeWidth={i % (step * 2) === 0 ? 2.5 : 1.5} />)

    if (i % (step * (max <= 60 ? 1 : 2)) === 0) {
      const labelR = r - 18
      const lx = cx + labelR * Math.cos(rad_a)
      const ly = cy + labelR * Math.sin(rad_a)
      ticks.push(
        <text key={`l${i}`} x={lx} y={ly} textAnchor="middle" dominantBaseline="central"
          fill="#E0E0E0" fontSize={max <= 60 ? 10 : 8} fontFamily="Oswald,sans-serif" fontWeight="600">
          {max <= 60 ? i : (i / 1000).toFixed(0)}
        </text>
      )
    }
  }

  return (
    <div className="relative" style={{ width: size, height: size }}>
      <svg width={size} height={size} viewBox={`0 0 ${size} ${size}`}>
        <defs>
          <radialGradient id={`bg${label}`} cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#2E2E2E" />
            <stop offset="85%" stopColor="#1E1E1E" />
            <stop offset="100%" stopColor="#141414" />
          </radialGradient>
          <radialGradient id={`bezel${label}`} cx="30%" cy="30%" r="70%">
            <stop offset="0%" stopColor="#E8E8EB" />
            <stop offset="50%" stopColor="#A8A8AD" />
            <stop offset="100%" stopColor="#6A6A6F" />
          </radialGradient>
          <linearGradient id={`glass${label}`} x1="0" y1="0" x2="1" y2="1">
            <stop offset="0%" stopColor="rgba(255,255,255,0.08)" />
            <stop offset="50%" stopColor="rgba(255,255,255,0)" />
            <stop offset="100%" stopColor="rgba(255,255,255,0.04)" />
          </linearGradient>
          <filter id={`shadow${label}`}>
            <feDropShadow dx="1" dy="1" stdDeviation="1" floodColor="#000" floodOpacity="0.5" />
          </filter>
        </defs>

        {/* Outer bezel */}
        <circle cx={cx} cy={cy} r={r + 14} fill={`url(#bezel${label})`} />
        <circle cx={cx} cy={cy} r={r + 11} fill="none" stroke="#3A3A3A" strokeWidth="1" />
        <circle cx={cx} cy={cy} r={r + 10} fill={`url(#bg${label})`} />

        {/* Colored arcs */}
        {warnStart && (
          <path
            d={describeArc(cx, cy, r - 3, startAngle + (warnStart - min) / (max - min) * range, endAngle)}
            fill="none" stroke="#E6A023" strokeWidth="3" opacity="0.6"
          />
        )}
        {redline && (
          <path
            d={describeArc(cx, cy, r - 3, startAngle + (redline - min) / (max - min) * range, endAngle)}
            fill="none" stroke="#E63946" strokeWidth="3" opacity="0.8"
          />
        )}

        {/* Ticks */}
        {ticks}

        {/* Center cap */}
        <circle cx={cx} cy={cy} r={6} fill="#8E8E93" />
        <circle cx={cx} cy={cy} r={4} fill="#D9D9D9" />

        {/* Needle */}
        <g filter={`url(#shadow${label})`}>
          <line x1={cx - 5 * Math.cos(rad + Math.PI)} y1={cy - 5 * Math.sin(rad + Math.PI)} x2={nx} y2={ny}
            stroke={redline && val >= redline ? '#E63946' : color} strokeWidth="2.5" strokeLinecap="round" />
          <circle cx={nx} cy={ny} r={2.5} fill={redline && val >= redline ? '#E63946' : color} />
        </g>

        {/* Glass reflection */}
        <circle cx={cx} cy={cy} r={r - 2} fill={`url(#glass${label})`} />

        {/* Unit text */}
        <text x={cx} y={cy + r * 0.35} textAnchor="middle" fill="#AAA" fontSize={8} fontFamily="Inter,sans-serif" fontWeight="500">
          {unit}
        </text>
      </svg>

      {/* Digital value */}
      <div className="absolute bottom-1 left-1/2 -translate-x-1/2 text-center">
        <div className="text-white font-bold tabular-nums" style={{ fontSize: size * 0.1, fontFamily: 'Oswald,sans-serif' }}>
          {max <= 60 ? Math.round(val) : Math.round(val)}
        </div>
      </div>
    </div>
  )
}

function describeArc(cx: number, cy: number, r: number, start: number, end: number) {
  const s = start * Math.PI / 180
  const e = end * Math.PI / 180
  const x1 = cx + r * Math.cos(s)
  const y1 = cy + r * Math.sin(s)
  const x2 = cx + r * Math.cos(e)
  const y2 = cy + r * Math.sin(e)
  const large = end - start > 180 ? 1 : 0
  return `M ${x1} ${y1} A ${r} ${r} 0 ${large} 1 ${x2} ${y2}`
}
