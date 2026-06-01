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
    x += velX * dt;
    y += velY * dt;

    // Si sale de los límites de la pantalla (800x600 con un margen por el sprite), se desactiva
    if (x < -85.0f || x > 800.0f || y < -85.0f || y > 600.0f) {
        activo = false;
        hide(); // Oculta el gráfico en la escena de Qt
    }

    setPosicion(x, y);
}
