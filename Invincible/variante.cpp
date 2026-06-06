#include "variante.h"
#include "jugador.h"
#include <cmath>

Variante::Variante(float x, float y, float vida, float masa,
                   float danio, uint16_t puntos)
    : Enemigo(x, y, vida, masa, danio, puntos)
    , estado(Estado::RODEAR)
    , angulo(0.0f)
    , tiempoEnEstado(0.0f)
{}

void Variante::update(float dt)
{
    moverse(dt);
}

void Variante::update(float dt, const Jugador& jugador)
{
    if (!isVisible()) return;

    tiempoEnEstado += dt;

    // Anti-bloqueo: si ataca más de 3s sin éxito vuelve a rodear
    if (estado == Estado::ATACAR && tiempoEnEstado > 3.0f) {
        setEstado(Estado::RODEAR);
    }

    float xObjetivo, yObjetivo;

    if (estado == Estado::RODEAR) {
        float radioAsedio  = 180.0f;
        float giroDinamico = angulo + (tiempoEnEstado * 0.5f);
        xObjetivo = jugador.getX() + std::cos(giroDinamico) * radioAsedio;
        yObjetivo = jugador.getY() + std::sin(giroDinamico) * radioAsedio;
    } else {
        xObjetivo = jugador.getX();
        yObjetivo = jugador.getY();
    }

    float dx   = xObjetivo - x;
    float dy   = yObjetivo - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > 5.0f) {
        float velActual = (estado == Estado::ATACAR) ? 120.0f : 80.0f;
        velX = (dx / dist) * velActual * 1.5f;
        velY = (dy / dist) * velActual * 1.5f;
    } else {
        velX = 0.0f;
        velY = 0.0f;
    }

    moverse(dt);
}

void Variante::moverse(float dt)
{
    x += velX * dt;
    y += velY * dt;
    limitarBordes(800.0f, 600.0f, 85.0f, 85.0f);
    setPosicion(x, y);
}

void Variante::setEstado(Estado nuevoEstado)
{
    if (estado != nuevoEstado) {
        estado        = nuevoEstado;
        tiempoEnEstado = 0.0f;
    }
}

Variante::Estado Variante::getEstado() const { return estado; }
void Variante::setAngulo(float nuevoAngulo)  { angulo = nuevoAngulo; }
