#ifndef VARIANTE_H
#define VARIANTE_H

#include "enemigo.h"

class Variante : public Enemigo {
public:
    enum class Estado { RODEAR, ATACAR };
    Variante(float x, float y, float vida, float masa, float danio, uint16_t puntos);

    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;

    // Métodos para el control táctico
    void setEstado(Estado nuevoEstado);
    Estado getEstado() const;
    void setAngulo(float nuevoAngulo);

private:
    Estado estado;
    float angulo;
    float tiempoEnEstado;
};

#endif // VARIANTE_H
