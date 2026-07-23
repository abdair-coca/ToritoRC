export function ConnectionStatus({ connected }: { connected: boolean }) {
  return (
    <div className="absolute top-4 left-1/2 -translate-x-1/2 flex items-center gap-2 bg-white/80 backdrop-blur rounded-full px-4 py-1.5 border border-stone-300 shadow-sm text-xs">
      <div className={`w-2 h-2 rounded-full ${connected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
      <span className={connected ? 'text-green-700' : 'text-red-700'}>
        {connected ? 'Conectado' : 'Desconectado'}
      </span>
    </div>
  )
}
