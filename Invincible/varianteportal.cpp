#include "varianteportal.h"

VariantePortal::VariantePortal(float x, float y, float vida, float masa, float danio, uint16_t puntos)
    : Enemigo(x, y, vida, masa, danio, puntos) {}

void VariantePortal::update(float dt) {
    moverse(dt);
}

void VariantePortal::update(float dt, const Jugador& jugador) {
    // Ignoramos la posición del jugador en tiempo real.
    // Su vector de velocidad (velX, velY) se le asigna solo una vez al momento de nacer.
    moverse(dt);
}

void VariantePortal::moverse(float dt) {

    // Si su velocidad es cero, está escondida esperando que el Agente abra un portal
    if (velX == 0.0f && velY == 0.0f) return;

    x += velX * dt;
    y += velY * dt;

    // Si sale de los límites de la pantalla
    if (x < -85.0f || x > 800.0f || y < -85.0f || y > 600.0f) {
        velX = 0.0f;
        velY = 0.0f;
        hide(); // Se oculta visualmente, pero sigue viva para el próximo portal
    }

    setPosicion(x, y);
}
