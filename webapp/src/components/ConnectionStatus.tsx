import { useState } from 'react'
import { wsService } from '../services/websocket'

export function ConnectionStatus({ connected }: { connected: boolean }) {
  const [showSettings, setShowSettings] = useState(false)
  const [url, setUrl] = useState('ws://192.168.4.1:81')

  const handleConnect = () => {
    wsService.connect(url)
    setShowSettings(false)
  }

  return (
    <div className="absolute top-4 left-1/2 -translate-x-1/2 flex flex-col items-center gap-2 z-50">
      <div 
        onClick={() => setShowSettings(!showSettings)}
        className="flex items-center gap-2 bg-white/90 backdrop-blur rounded-full px-4 py-1.5 border border-stone-300 shadow-md text-xs cursor-pointer hover:bg-stone-100 transition-colors"
      >
        <div className={`w-2.5 h-2.5 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
        <span className={`font-medium ${connected ? 'text-green-700' : 'text-red-700'}`}>
          {connected ? 'Conectado (ws://192.168.4.1:81)' : 'Desconectado - Configurar IP ⚙️'}
        </span>
      </div>

      {showSettings && (
        <div className="bg-stone-900/90 text-white backdrop-blur p-4 rounded-xl shadow-2xl border border-stone-700 text-xs flex flex-col gap-3 w-72 animate-in fade-in zoom-in-95">
          <div className="font-semibold text-stone-200">Dirección del Servidor WebSocket</div>
          <input
            type="text"
            value={url}
            onChange={(e) => setUrl(e.target.value)}
            className="bg-stone-800 border border-stone-600 rounded px-3 py-1.5 text-stone-100 text-xs focus:outline-none focus:border-amber-500"
            placeholder="ws://192.168.4.1:81"
          />
          <div className="flex gap-2 justify-end">
            <button
              onClick={() => setShowSettings(false)}
              className="px-3 py-1 bg-stone-700 hover:bg-stone-600 rounded text-stone-300 transition-colors"
            >
              Cancelar
            </button>
            <button
              onClick={handleConnect}
              className="px-3 py-1 bg-amber-500 hover:bg-amber-400 text-stone-950 font-semibold rounded transition-colors"
            >
              Reconectar
            </button>
          </div>
        </div>
      )}
    </div>
  )
}
