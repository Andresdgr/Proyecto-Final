#include "portal.h"
#include <cmath>

// ── Constructor Nivel 1 — MAS ────────────────────────────────────
Portal::Portal(float x, float y, float amplitud, float omegaMAS)
    : Entidad(x, y, 1.0f, 0.0f)
    , tiempoVida(0.0f)
    , tiempoMovimiento(0.0f)
    , amplitud(amplitud)
    , omegaMAS(omegaMAS)
    , radioActual(amplitud)
    , cx(x)
    , cy(y)
    , radioOrbital(0.0f)
    , omegaOrbital(0.0f)
    , fase(0.0f)
    , esPuntoSpawn(false)
{
}

// ── Constructor Nivel 2 — Orbital ────────────────────────────────
Portal::Portal(float cx, float cy, float radioOrbital,
               float omegaOrbital, float fase)
    : Entidad(cx + radioOrbital * std::cos(fase),
              cy + radioOrbital * std::sin(fase),
              1.0f, 0.0f)
    , tiempoVida(0.0f)
    , tiempoMovimiento(0.0f)
    , amplitud(0.0f)
    , omegaMAS(0.0f)
    , radioActual(30.0f)
    , cx(cx)
    , cy(cy)
    , radioOrbital(radioOrbital)
    , omegaOrbital(omegaOrbital)
    , fase(fase)
    , esPuntoSpawn(true)
{
}

// ── Destructor ───────────────────────────────────────────────────
Portal::~Portal()
{
}

// ── update básico ────────────────────────────────────────────────
void Portal::update(float dt)
{
    (void)dt;
    if (!activo) return;
}

// ── updateConPhysics ─────────────────────────────────────────────
void Portal::updateConPhysics(float dt, const PhysicsEngine& physics)
{
    if (!activo) return;

    tiempoVida       += dt;
    tiempoMovimiento += dt;

    if (!esPuntoSpawn) {
        // F2 — MAS: radio de zona de daño oscilante
        radioActual = physics.MAS(amplitud, omegaMAS, tiempoVida);
        if (radioActual < 20.0f) radioActual = 20.0f;

        // Movimiento por escena — dos MAS combinados
        // Genera trayectoria elíptica impredecible
        x = cx + std::cos(tiempoMovimiento * 0.4f) * 200.0f;
        y = cy + std::sin(tiempoMovimiento * 0.3f) * 100.0f;

        // Límites de pantalla
        if (x < 30.0f)  x = 30.0f;
        if (x > 750.0f) x = 750.0f;
        if (y < 30.0f)  y = 30.0f;
        if (y > 450.0f) y = 450.0f;

    } else {
        // F5 — Orbital
        QPointF nuevaPos = physics.orbital(
            cx, cy, radioOrbital, omegaOrbital, fase, tiempoVida);
        x = static_cast<float>(nuevaPos.x());
        y = static_cast<float>(nuevaPos.y());
    }
}

// ── jugadorTocaBorde ─────────────────────────────────────────────
bool Portal::jugadorTocaBorde(float xJ, float yJ,
                              float anchoJug, float altoJug) const
{
    if (!activo) return false;

    float cxJ = xJ + anchoJug / 2.0f;
    float cyJ = yJ + altoJug  / 2.0f;

    float dx = cxJ - x;
    float dy = cyJ - y;
    float distancia = std::sqrt(dx * dx + dy * dy);

    float margenBorde = anchoJug / 2.0f;
    return distancia < (radioActual + margenBorde);
}

// ── Getters ──────────────────────────────────────────────────────
float Portal::getRadioActual() const { return radioActual; }
bool  Portal::isPuntoSpawn()   const { return esPuntoSpawn; }
