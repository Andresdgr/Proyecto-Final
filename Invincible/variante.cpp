#include "variante.h"

#include "jugador.h"
#include <cmath>

Variante::Variante(float x, float y, float vida, float masa, float danio, uint16_t puntos)
    : Enemigo(x, y, vida, masa, danio, puntos) {}

void Variante::update(float dt) {
    moverse(dt);
}

void Variante::update(float dt, const Jugador& jugador) {
    // Si la variante está esperando turno (oculta), no hace nada.
    if (!isVisible()) return;
    // Seguir al jugador
    float dx = jugador.getX() - x;
    float dy = jugador.getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    velX = (dx / dist) * 120.0f;  // velocidad de 50 px/s
    velY = (dy / dist) * 120.0f;
    moverse(dt);
}

void Variante::moverse(float dt) {
    x += velX * dt;
    y += velY * dt;
    limitarBordes(800.0f, 600.0f, 85.0f, 85.0f);
    setPosicion(x, y);  // Sincroniza con QGraphicsPixmapItem
}

