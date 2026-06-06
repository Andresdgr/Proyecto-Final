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
    , yJugadorPercibida(300.0f)
    , direccionPercibida(0)
    , jugadorCercano(false)
    , golpeado(false)
    , tiempoGolpe(0.0f)
    , xInicioGolpe(0.0f)
    , yInicioGolpe(0.0f)
    , velXGolpe(0.0f)
    , velYGolpe(0.0f)
    , velocidadBase(120.0f)
    , cooldownEvasionBase(2.0f)
    , tiempoEntreEvasiones(0.0f)
    , cooldownEvasion(0.0f)
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

    // Reducir cooldown de evasión
    if (cooldownEvasion > 0.0f) {
        cooldownEvasion -= dt;
        if (cooldownEvasion < 0.0f) cooldownEvasion = 0.0f;
    }

    // Si está golpeado ejecutar parábola
    if (golpeado) {
        tiempoGolpe += dt;
        float gravedad = 300.0f;
        x = xInicioGolpe + velXGolpe * tiempoGolpe;
        y = yInicioGolpe + velYGolpe * tiempoGolpe
            + 0.5f * gravedad * tiempoGolpe * tiempoGolpe;

        if (x < 0.0f)   x = 0.0f;
        if (x > 750.0f) x = 750.0f;

        if (y > 480.0f) {
            y = 480.0f;
            golpeado     = false;
            tiempoGolpe  = 0.0f;
            estadoActual = EstadoIA::AGRESIVO;
            cooldownEvasion = cooldownEvasionBase * 0.75f;
        }

        return;
    }

    // Ciclo normal de IA
    percibir(jugador);
    razonar();
    actuar(dt);
    aprender();
}
// ════════════════════════════════════════════════════════════════
//  1. PERCEPCIÓN
// ════════════════════════════════════════════════════════════════
void AngstromLevy::percibir(const Jugador& jugador)
{
    xJugadorPercibida    = jugador.getX();
    yJugadorPercibida    = jugador.getY(); // ← agregar
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
    // Si está golpeado no razona
    if (estadoActual == EstadoIA::GOLPEADO) return;

    // Reducir cooldown de evasión
    if (cooldownEvasion > 0.0f) {
        // No puede evadir todavía — ser agresivo
        estadoActual = EstadoIA::AGRESIVO;
        return;
    }

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
    case EstadoIA::AGRESIVO:   moverse(dt);        break;
    case EstadoIA::EVASIVO:    teleportarse();      break;
    case EstadoIA::DEFENSIVO:  esperarYDaniar();    break;
    case EstadoIA::GOLPEADO:   /* manejado en update */ break;
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
    float offsetX = (xJugadorPercibida > x) ? -radioPortal : radioPortal;
    float offsetY = (yJugadorPercibida > y) ? -radioPortal : radioPortal;
    x = xJugadorPercibida + offsetX;
    y = yJugadorPercibida + offsetY;

    if (x < 0.0f)   x = 0.0f;
    if (x > 750.0f) x = 750.0f;
    if (y < 0.0f)   y = 0.0f;
    if (y > 480.0f) y = 480.0f;

    // Cooldown de 2 segundos entre evasiones
    cooldownEvasion = cooldownEvasionBase;
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

void AngstromLevy::esperarYDaniar()
{
    // En estado defensivo Levy se mueve lateralmente
    // para esquivar y buscar un ángulo diferente
    float tiempoLateral = contadorInteracciones * 0.016f;
    velX = std::sin(tiempoLateral * 2.0f) * 80.0f;
    velY = 0.0f;
}

void AngstromLevy::recibirImpacto(float dirX)
{
    golpeado       = true;
    tiempoGolpe    = 0.0f;
    xInicioGolpe   = x;
    yInicioGolpe   = y;

    // Velocidad inicial del golpe
    // dirX: -1 si el jugador está a la derecha (Levy sale a la izquierda)
    //        1 si el jugador está a la izquierda (Levy sale a la derecha)
    velXGolpe = dirX * 200.0f;  // velocidad horizontal
    velYGolpe = -150.0f;        // siempre sube primero (parábola)

    estadoActual = EstadoIA::GOLPEADO;
}
void AngstromLevy::moverse(float dt)
{
    float dx = xJugadorPercibida - x;
    float dy = yJugadorPercibida - y;

    float distancia = std::sqrt(dx * dx + dy * dy);
    if (distancia < 5.0f) return;

    float velocidad = velocidadBase * agresividad;
    x += (dx / distancia) * velocidad * dt;
    y += (dy / distancia) * velocidad * dt;

    if (x < 0.0f)   x = 0.0f;
    if (x > 750.0f) x = 750.0f;
    if (y < 0.0f)   y = 0.0f;
    if (y > 480.0f) y = 480.0f;
}

void AngstromLevy::aplicarDificultad(float agr, float velBase, float cdBase)
{
    agresividad       = agr;
    velocidadBase     = velBase;
    cooldownEvasionBase = cdBase;
}
