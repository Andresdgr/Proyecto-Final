#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "entidad.h"
#include <cstdint>

class Jugador;

class Enemigo : public Entidad {

public:

    Enemigo(float x, float y, float vida, float masa,
            float danio, uint16_t puntosAlDerrotar);

    virtual ~Enemigo();

    // Permite que ambas versiones de update coexistan
    using Entidad::update;

    virtual void update(float dt, const Jugador& jugador) = 0;

    virtual void moverse(float dt) = 0;

    float    getDanio()             const;
    uint16_t getPuntosAlDerrotar()  const;

protected:

    float    danio;
    uint16_t puntosAlDerrotar;

};

#endif
