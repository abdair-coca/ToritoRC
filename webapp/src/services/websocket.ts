import type { ControlCommand, Telemetry } from '../types/protocol'

type CmdCallback = (cmd: ControlCommand) => void
type TeleCallback = (data: Telemetry) => void

class WebSocketService {
  private ws: WebSocket | null = null
  private url = 'ws://192.168.4.1:81'
  private cmdCallbacks: CmdCallback[] = []
  private teleCallbacks: TeleCallback[] = []
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null
  private _connected = false

  get connected() { return this._connected }

  connect(url?: string) {
    if (url) this.url = url
    if (this.ws) this.ws.close()

    this.ws = new WebSocket(this.url)
    this.ws.onopen = () => { this._connected = true }
    this.ws.onclose = () => {
      this._connected = false
      this.scheduleReconnect()
    }
    this.ws.onmessage = (e) => {
      try {
        const data = JSON.parse(e.data) as Telemetry
        this.teleCallbacks.forEach(cb => cb(data))
      } catch { /* ignore */ }
    }
  }

  send(cmd: ControlCommand) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(cmd))
    }
  }

  onCommand(cb: CmdCallback) { this.cmdCallbacks.push(cb) }
  onTelemetry(cb: TeleCallback) { this.teleCallbacks.push(cb) }

  private scheduleReconnect() {
    if (this.reconnectTimer) return
    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null
      this.connect()
    }, 2000)
  }

  disconnect() {
    if (this.reconnectTimer) clearTimeout(this.reconnectTimer)
    this.ws?.close()
    this.ws = null
    this._connected = false
  }
}

export const wsService = new WebSocketService()
