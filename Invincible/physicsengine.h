#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include "difficultyconfig.h"
#include <QPointF>

class PhysicsEngine {

public:

    PhysicsEngine(const DifficultyConfig& config);

    // ── F1: Movimiento parabólico ────────────────────────────
    QPointF parabolica(float x0, float y0,
                       float v0x, float v0y,
                       float t) const;

    // ── F2: Movimiento armónico simple ───────────────────────
    float MAS(float A, float omega, float t) const;

    // ── F3: Aceleración progresiva ───────────────────────────
    float acelerarClon(float velActual,
                       float aceleracion,
                       float velMax,
                       float dt) const;

    // ── F4: Rebote contra borde ──────────────────────────────
    float reboteBorde(float velActual,
                      float coefRestitucion) const;

    // ── F5: Movimiento orbital ───────────────────────────────
    QPointF orbital(float cx, float cy,
                    float r, float omega,
                    float fase, float t) const;

    // ── Getters y setter ─────────────────────────────────────
    float getGravedad() const;
    float getDt()       const;
    void  setDt(float dt);

private:

    float gravedadG;
    float dt;

};

#endif
