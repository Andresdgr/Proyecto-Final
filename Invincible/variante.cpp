#include "variante.h"
#include "jugador.h"
#include <cmath>

// ── Constructor ─────────────────────────────────────────────────
Variante::Variante(float x, float y, float vida, float masa,
                   float danio, uint16_t puntos)
    : Enemigo(x, y, vida, masa, danio, puntos)
    , estado(Estado::RODEAR)
    , angulo(0.0f)
    , tiempoEnEstado(0.0f)
{}

// ── update sin jugador ───────────────────────────────────────────
void Variante::update(float dt)
{
    (void)dt;
}

// ── update con jugador — orquesta percepción y acción ────────────
void Variante::update(float dt, const Jugador& jugador)
{
    if (!activo) return;

    float xJ = jugador.getX();
    float yJ = jugador.getY();

    float dx   = xJ - x;
    float dy   = yJ - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    tiempoEnEstado += dt;

    // ── Razonamiento — decide estado según distancia ──────────
    if (dist < 120.0f) {
        // Cerca del jugador — atacar
        if (estado != Estado::ATACAR) {
            estado         = Estado::ATACAR;
            tiempoEnEstado = 0.0f;
        }
    } else {
        // Lejos — rodear
        if (estado != Estado::RODEAR) {
            estado         = Estado::RODEAR;
            tiempoEnEstado = 0.0f;
        }
    }

    // ── Acción ────────────────────────────────────────────────
    switch (estado) {
    case Estado::RODEAR:
        rodear(dt, xJ, yJ);
        break;
    case Estado::ATACAR:
        atacar(dt, xJ, yJ);
        break;
    }
}

// ── moverse ──────────────────────────────────────────────────────
void Variante::moverse(float dt)
{
    (void)dt;
}

// ── rodear — movimiento circular alrededor del jugador ───────────
void Variante::rodear(float dt, float xJ, float yJ)
{
    angulo += 1.2f * dt;

    float radio    = 150.0f;
    float destinoX = xJ + std::cos(angulo) * radio;
    float destinoY = yJ + std::sin(angulo) * radio;

    float dx   = destinoX - x;
    float dy   = destinoY - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    float vel = 90.0f;

    if (dist > 2.0f) {
        x += (dx / dist) * vel * dt;
        y += (dy / dist) * vel * dt;
    }

    // Límites de pantalla
    x = std::max(0.0f, std::min(x, 740.0f));
    y = std::max(0.0f, std::min(y, 480.0f));
}

// ── atacar — se lanza directo hacia el jugador ───────────────────
void Variante::atacar(float dt, float xJ, float yJ)
{
    float dx   = xJ - x;
    float dy   = yJ - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    float vel = 160.0f;

    if (dist > 5.0f) {
        x += (dx / dist) * vel * dt;
        y += (dy / dist) * vel * dt;
    }

    // Límites de pantalla
    x = std::max(0.0f, std::min(x, 740.0f));
    y = std::max(0.0f, std::min(y, 480.0f));
}

// ── Getters y setters ────────────────────────────────────────────
void Variante::setEstado(Estado nuevoEstado) { estado = nuevoEstado; }
Variante::Estado Variante::getEstado() const { return estado; }
void Variante::setAngulo(float nuevoAngulo)  { angulo = nuevoAngulo; }
