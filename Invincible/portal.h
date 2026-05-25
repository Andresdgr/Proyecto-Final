#ifndef PORTAL_H
#define PORTAL_H

#include "entidad.h"
#include "physicsengine.h"

class Portal : public Entidad {

public:

    // ── Constructor Nivel 1 — MAS ────────────────────────────
    Portal(float x, float y, float amplitud, float omegaMAS);

    // ── Constructor Nivel 2 — Orbital ────────────────────────
    Portal(float cx, float cy, float radioOrbital,
           float omegaOrbital, float fase);

    virtual ~Portal();

    // ── Ciclo de vida por tick ───────────────────────────────
    void update(float dt) override;
    void updateConPhysics(float dt, const PhysicsEngine& physics);

    // ── Consulta de zona de daño ─────────────────────────────
    bool jugadorEnZonaDanio(float xJ, float yJ) const;

    // ── Getters propios ──────────────────────────────────────
    float getRadioActual()  const;
    bool  isPuntoSpawn()    const;

private:

    // ── Acumulador de tiempo ─────────────────────────────────
    float tiempoVida;

    // ── Nivel 1: MAS ─────────────────────────────────────────
    float amplitud;
    float omegaMAS;
    float radioActual;

    // ── Nivel 2: Orbital ─────────────────────────────────────
    float cx;
    float cy;
    float radioOrbital;
    float omegaOrbital;
    float fase;

    // ── Control de comportamiento ────────────────────────────
    bool  esPuntoSpawn;

};

#endif
