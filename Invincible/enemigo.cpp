#include "enemigo.h"

Enemigo::Enemigo(float x, float y, float vida, float masa,
                 float danio, uint16_t puntosAlDerrotar)
    : Entidad(x, y, vida, masa)
    , danio(danio)
    , puntosAlDerrotar(puntosAlDerrotar)
{
}

Enemigo::~Enemigo()
{
}

float    Enemigo::getDanio()            const { return danio;            }
uint16_t Enemigo::getPuntosAlDerrotar() const { return puntosAlDerrotar; }

void Enemigo::setDanio(float nuevoDanio)
{
    if (nuevoDanio >= 0.0f) danio = nuevoDanio;
}
