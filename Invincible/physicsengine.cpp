#include "physicsengine.h"
#include <cmath>
#include <stdexcept>

PhysicsEngine::PhysicsEngine(const DifficultyConfig& config)
    : gravedadG(config.gravedadG)
    , dt(0.016f)
{
}

// ── F1: Parábola ─────────────────────────────────────────────────
// x(t) = x0 + v0x · t
// y(t) = y0 + v0y · t − ½ · g · t²
QPointF PhysicsEngine::parabolica(float x0, float y0,
                                  float v0x, float v0y,
                                  float t) const
{
    if (t < 0.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::parabolica — t no puede ser negativo");
    }

    float x = x0 + v0x * t;
    float y = y0 + v0y * t - 0.5f * gravedadG * t * t;
    return QPointF(x, y);
}

// ── F2: MAS ──────────────────────────────────────────────────────
// radio(t) = A · sin(ω · t)
float PhysicsEngine::MAS(float A, float omega, float t) const
{
    if (A < 0.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::MAS — amplitud no puede ser negativa");
    }

    return A * std::sin(omega * t);
}

// ── F3: Aceleración progresiva ───────────────────────────────────
// v(t) = v0 + a · dt   limitado a velMax
float PhysicsEngine::acelerarClon(float velActual,
                                  float aceleracion,
                                  float velMax,
                                  float dt) const
{
    if (velMax <= 0.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::acelerarClon — velMax debe ser positivo");
    }

    float nuevaVel = velActual + aceleracion * dt;
    if (nuevaVel > velMax) nuevaVel = velMax;
    return nuevaVel;
}

// ── F4: Rebote contra borde ──────────────────────────────────────
// vf = −e · vi
float PhysicsEngine::reboteBorde(float velActual,
                                 float coefRestitucion) const
{
    if (coefRestitucion < 0.0f || coefRestitucion > 1.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::reboteBorde — coefRestitucion debe estar entre 0 y 1");
    }

    return -coefRestitucion * velActual;
}

// ── F5: Movimiento orbital ───────────────────────────────────────
// x(t) = cx + r · cos(ω · t + φ)
// y(t) = cy + r · sin(ω · t + φ)
QPointF PhysicsEngine::orbital(float cx, float cy,
                               float r, float omega,
                               float fase, float t) const
{
    if (r < 0.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::orbital — radio no puede ser negativo");
    }

    float x = cx + r * std::cos(omega * t + fase);
    float y = cy + r * std::sin(omega * t + fase);
    return QPointF(x, y);
}

// ── Getters y setter ─────────────────────────────────────────────
float PhysicsEngine::getGravedad() const { return gravedadG; }
float PhysicsEngine::getDt()       const { return dt;        }

void PhysicsEngine::setDt(float nuevoDt)
{
    if (nuevoDt <= 0.0f) {
        throw std::invalid_argument(
            "PhysicsEngine::setDt — dt debe ser positivo");
    }
    dt = nuevoDt;
}
