import { useRef, useCallback } from 'react'

type SynthParams = {
  rpm: number
  gear: number
  brake: boolean
  blinker: boolean
}

export function useAudio() {
  const ctxRef = useRef<AudioContext | null>(null)
  const oscRef = useRef<OscillatorNode | null>(null)
  const gainRef = useRef<GainNode | null>(null)
  const blinkerInterval = useRef<ReturnType<typeof setInterval> | null>(null)
  const prevBlinker = useRef(false)

  const getCtx = useCallback(() => {
    if (!ctxRef.current) {
      ctxRef.current = new AudioContext()
    }
    return ctxRef.current
  }, [])

  const updateEngine = useCallback(({ rpm, gear, brake }: SynthParams) => {
    const ctx = getCtx()
    if (ctx.state === 'suspended') ctx.resume()

    if (gear === 0 || rpm === 0) {
      oscRef.current?.stop()
      oscRef.current = null
      gainRef.current = null
      return
    }

    if (!oscRef.current) {
      oscRef.current = ctx.createOscillator()
      gainRef.current = ctx.createGain()
      gainRef.current.gain.value = 0
      oscRef.current.connect(gainRef.current)
      gainRef.current.connect(ctx.destination)
      oscRef.current.start()
    }

    const freq = 40 + (rpm / 8000) * 260
    const vol = brake ? 0.02 : 0.04 + (rpm / 8000) * 0.06

    oscRef.current.type = 'sawtooth'
    oscRef.current.frequency.setTargetAtTime(freq, ctx.currentTime, 0.05)
    gainRef.current!.gain.setTargetAtTime(vol, ctx.currentTime, 0.05)
  }, [getCtx])

  const updateBlinker = useCallback((active: boolean) => {
    if (active && !prevBlinker.current) {
      const ctx = getCtx()
      blinkerInterval.current = setInterval(() => {
        const osc = ctx.createOscillator()
        const gain = ctx.createGain()
        osc.frequency.value = 800
        gain.gain.value = 0.08
        osc.connect(gain).connect(ctx.destination)
        osc.start()
        osc.stop(ctx.currentTime + 0.05)
      }, 600)
    }
    if (!active && prevBlinker.current) {
      if (blinkerInterval.current) clearInterval(blinkerInterval.current)
      blinkerInterval.current = null
    }
    prevBlinker.current = active
  }, [getCtx])

  return { updateEngine, updateBlinker }
}
