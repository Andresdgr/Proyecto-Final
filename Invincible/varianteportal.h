#ifndef VARIANTEPORTAL_H
#define VARIANTEPORTAL_H

#include "enemigo.h"

class VariantePortal : public Enemigo {
public:
    VariantePortal(float x, float y, float vida, float masa, float danio, uint16_t puntos);

    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;
};

#endif // VARIANTEPORTAL_H
