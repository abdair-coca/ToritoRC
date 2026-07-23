import { useState } from 'react'
import { wsService } from '../services/websocket'

export function ConnectionStatus({ connected }: { connected: boolean }) {
  const [showSettings, setShowSettings] = useState(false)
  const [url, setUrl] = useState('ws://192.168.4.1:81')
  const isHttps = typeof window !== 'undefined' && window.location.protocol === 'https:'

  const handleConnect = () => {
    wsService.connect(url)
    setShowSettings(false)
  }

  return (
    <div className="absolute top-4 left-1/2 -translate-x-1/2 flex flex-col items-center gap-2 z-50 max-w-sm w-full px-4">
      <div 
        onClick={() => setShowSettings(!showSettings)}
        className="flex items-center gap-2 bg-white/95 backdrop-blur rounded-full px-4 py-1.5 border border-stone-300 shadow-md text-xs cursor-pointer hover:bg-stone-100 transition-colors"
      >
        <div className={`w-2.5 h-2.5 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
        <span className={`font-medium ${connected ? 'text-green-700' : 'text-red-700'}`}>
          {connected ? 'Conectado a ToritoRC 🚘' : 'Desconectado - Toca para ayuda / IP ⚙️'}
        </span>
      </div>

      {showSettings && (
        <div className="bg-stone-900/95 text-white backdrop-blur p-4 rounded-xl shadow-2xl border border-stone-700 text-xs flex flex-col gap-3 w-full max-w-xs animate-in fade-in zoom-in-95">
          <div className="font-semibold text-stone-200">Estado de Conexión WebSocket</div>
          
          {isHttps && !connected && (
            <div className="bg-amber-950/80 border border-amber-500/50 p-2.5 rounded text-amber-200 text-[11px] leading-relaxed">
              <strong>⚠️ Sitio cargado por HTTPS (Vercel):</strong>
              <p className="mt-1 text-amber-300">
                Los navegadores bloquean <code>ws://</code> desde páginas HTTPS. 
              </p>
              <ul className="list-disc ml-3.5 mt-1 space-y-0.5 text-stone-300">
                <li><strong>Android/Chrome:</strong> Toca los 3 puntos <code>⋮</code> ➔ Info del sitio ➔ Permisos ➔ Contenido no seguro (Permitir).</li>
                <li><strong>iPhone/Safari:</strong> Toca <code>aA</code> en la URL ➔ Ajustes de sitio ➔ Permitir red local.</li>
                <li><strong>PC/Chrome:</strong> Toca el icono 🎛️ a la izquierda del URL ➔ Ajustes de sitio ➔ Contenido no seguro (Permitir).</li>
              </ul>
            </div>
          )}

          <div>
            <label className="block text-stone-400 mb-1 text-[11px]">Servidor ToritoRC (ESP8266):</label>
            <input
              type="text"
              value={url}
              onChange={(e) => setUrl(e.target.value)}
              className="w-full bg-stone-800 border border-stone-600 rounded px-3 py-1.5 text-stone-100 text-xs focus:outline-none focus:border-amber-500 font-mono"
              placeholder="ws://192.168.4.1:81"
            />
          </div>

          <div className="flex gap-2 justify-end pt-1">
            <button
              onClick={() => setShowSettings(false)}
              className="px-3 py-1 bg-stone-700 hover:bg-stone-600 rounded text-stone-300 transition-colors"
            >
              Cerrar
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
