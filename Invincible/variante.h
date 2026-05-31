#ifndef VARIANTE_H
#define VARIANTE_H

#include "enemigo.h"

class Variante : public Enemigo {
public:
    Variante(float x, float y, float vida, float masa, float danio, uint16_t puntos);

    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;
};

#endif // VARIANTE_H
