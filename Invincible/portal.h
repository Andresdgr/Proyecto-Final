#ifndef PORTAL_H
#define PORTAL_H

#include "entidad.h"
#include "physicsengine.h"

class Portal : public Entidad {

public:

    Portal(float x, float y, float amplitud, float omegaMAS);

    Portal(float cx, float cy, float radioOrbital,
           float omegaOrbital, float fase);

    virtual ~Portal();

    void update(float dt) override;
    void updateConPhysics(float dt, const PhysicsEngine& physics);

    bool jugadorTocaBorde(float xJ, float yJ,
                          float anchoJug, float altoJug) const;

    float getRadioActual() const;
    bool  isPuntoSpawn()   const;

private:

    float tiempoVida;
    float tiempoMovimiento;

    float amplitud;
    float omegaMAS;
    float radioActual;

    float cx;
    float cy;
    float radioOrbital;
    float omegaOrbital;
    float fase;

    bool  esPuntoSpawn;

};

#endif
