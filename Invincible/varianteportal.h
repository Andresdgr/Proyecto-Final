#ifndef VARIANTEPORTAL_H
#define VARIANTEPORTAL_H

#include "enemigo.h"

class VariantePortal : public Enemigo {
private:
    int zonaOrigen = -1;
    bool impactoAlJugador = false;

public:
    VariantePortal(float x, float y, float vida, float masa, float danio, uint16_t puntos);

    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;

    void setZonaOrigen(int zona) { zonaOrigen = zona; }
    int getZonaOrigen() const { return zonaOrigen; }
    void setImpacto(bool impacto) { impactoAlJugador = impacto; }
    bool getImpacto() const { return impactoAlJugador; }
};

#endif // VARIANTEPORTAL_H
