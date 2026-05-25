#include "angstromlevy.h"
#include "jugador.h"
#include <cmath>
#include <cstdlib>
#include <stdexcept>

// ── Constructor ─────────────────────────────────────────────────
AngstromLevy::AngstromLevy(float x, float y)
    : Enemigo(x, y, 150.0f, 90.0f, 18.0f, 1000)
    , estadoActual(EstadoIA::AGRESIVO)
    , agresividad(1.0f)
    , radioPortal(RADIO_PORTAL_BASE)
    , contadorInteracciones(0)
    , distanciaAlJugador(0.0f)
    , vidaJugadorPercibida(100.0f)
    , xJugadorPercibida(0.0f)
    , direccionPercibida(0)
    , jugadorCercano(false)
{
}

// ── Destructor ──────────────────────────────────────────────────
AngstromLevy::~AngstromLevy()
{
    // deque se libera automáticamente
}

// ── update sin jugador — satisface la cadena de herencia ─────────
void AngstromLevy::update(float dt)
{
    // Esta versión existe para satisfacer la cadena de herencia.
    // AngstromLevy siempre se actualiza con update(dt, jugador).
    // GameEngine nunca llama esta versión directamente.
    (void)dt;
}

// ── update con jugador — orquesta los 4 componentes ─────────────
void AngstromLevy::update(float dt, const Jugador& jugador)
{
    if (!activo) return;

    percibir(jugador);   // 1. Sensa el entorno
    razonar();           // 2. Decide el estado
    actuar(dt);          // 3. Ejecuta la acción
    aprender();          // 4. Almacena y aprende
}

// ── moverse — desplazamiento hacia el jugador ────────────────────
void AngstromLevy::moverse(float dt)
{
    float direccion = (xJugadorPercibida > x) ? 1.0f : -1.0f;
    float velocidad = 2.5f * agresividad;
    x += direccion * velocidad * dt;
}

// ════════════════════════════════════════════════════════════════
//  1. PERCEPCIÓN
// ════════════════════════════════════════════════════════════════
void AngstromLevy::percibir(const Jugador& jugador)
{
    xJugadorPercibida    = jugador.getX();
    vidaJugadorPercibida = jugador.getVida();

    float dx = jugador.getX() - x;
    float dy = jugador.getY() - y;
    distanciaAlJugador = std::sqrt(dx * dx + dy * dy);

    float velX = jugador.getVelX();
    if      (velX >  0.5f) direccionPercibida =  1;
    else if (velX < -0.5f) direccionPercibida = -1;
    else                   direccionPercibida =  0;

    jugadorCercano = (distanciaAlJugador < UMBRAL_DISTANCIA_EVASION);
}

// ════════════════════════════════════════════════════════════════
//  2. RAZONAMIENTO — máquina de estados con prioridades
// ════════════════════════════════════════════════════════════════
void AngstromLevy::razonar()
{
    // Prioridad 1 — Levy en peligro crítico
    if (vida < vidaMaxima * UMBRAL_VIDA_DEFENSIVO) {
        estadoActual = EstadoIA::DEFENSIVO;
        return;
    }

    // Prioridad 2 — jugador demasiado cerca
    if (jugadorCercano) {
        estadoActual = EstadoIA::EVASIVO;
        return;
    }

    // Prioridad 3 — estado por defecto
    estadoActual = EstadoIA::AGRESIVO;
}

// ════════════════════════════════════════════════════════════════
//  3. ACCIÓN — delega según el estado activo
// ════════════════════════════════════════════════════════════════
void AngstromLevy::actuar(float dt)
{
    switch (estadoActual) {
    case EstadoIA::AGRESIVO:  moverse(dt);     break;
    case EstadoIA::EVASIVO:   teleportarse();  break;
    case EstadoIA::DEFENSIVO: esperarYDañar(); break;
    default:
        throw std::logic_error(
            "EstadoIA desconocido en AngstromLevy::actuar()");
    }
    contadorInteracciones++;
}

// ════════════════════════════════════════════════════════════════
//  4. APRENDIZAJE — ventana deslizante de direcciones
// ════════════════════════════════════════════════════════════════
void AngstromLevy::aprender()
{
    historialDirecciones.push_front(direccionPercibida);

    if (historialDirecciones.size() > CAPACIDAD_HISTORIAL) {
        historialDirecciones.pop_back();
    }

    if (contadorInteracciones % INTERVALO_APRENDIZAJE == 0
        && !historialDirecciones.empty()) {
        ajustarParametros();
    }
}

// ── Acciones ────────────────────────────────────────────────────
void AngstromLevy::teleportarse()
{
    float offset = (xJugadorPercibida > x) ? -radioPortal : radioPortal;
    x = xJugadorPercibida + offset;
}

void AngstromLevy::esperarYDañar()
{
    velX = 0.0f;
    velY = 0.0f;
}

// ── Helpers del aprendizaje ──────────────────────────────────────
float AngstromLevy::calcularFrecuenciaDerecha() const
{
    if (historialDirecciones.empty()) return 0.5f;

    uint8_t contDerecha = 0;
    for (int8_t d : historialDirecciones) {
        if (d == 1) contDerecha++;
    }

    return static_cast<float>(contDerecha)
           / static_cast<float>(historialDirecciones.size());
}

void AngstromLevy::ajustarParametros()
{
    float freqDer = calcularFrecuenciaDerecha();

    if (freqDer > FRECUENCIA_UMBRAL) {
        radioPortal = RADIO_PORTAL_AJUSTADO;
    } else if (freqDer < (1.0f - FRECUENCIA_UMBRAL)) {
        radioPortal = RADIO_PORTAL_AJUSTADO;
    } else {
        radioPortal = RADIO_PORTAL_BASE;
    }

    agresividad = 1.0f + (contadorInteracciones / 300.0f);
    if (agresividad > AGRESIVIDAD_MAX) {
        agresividad = AGRESIVIDAD_MAX;
    }
}

// ── Getter del estado (para DebugOverlay) ───────────────────────
AngstromLevy::EstadoIA AngstromLevy::getEstadoActual() const
{
    return estadoActual;
}
