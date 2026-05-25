#include "entidad.h"

// ── Constructor ─────────────────────────────────────────────────
Entidad::Entidad(float x, float y, float vida, float masa)
    : x(x)
    , y(y)
    , velX(0.0f)
    , velY(0.0f)
    , vida(vida)
    , vidaMaxima(vida)
    , masa(masa)
    , activo(true)
{
}

// ── Destructor ──────────────────────────────────────────────────
Entidad::~Entidad()
{
    // garantiza la cadena correcta de destrucción en subclases.
}

// ── recibirDanio ────────────────────────────────────────────────
void Entidad::recibirDanio(float cantidad)
{
    if (!activo) return;

    vida -= cantidad;

    if (vida <= 0.0f) {
        vida   = 0.0f;
        activo = false;
    }
}

// ── Getters ─────────────────────────────────────────────────────
float Entidad::getX()          const { return x;          }
float Entidad::getY()          const { return y;          }
float Entidad::getVelX()       const { return velX;       }
float Entidad::getVelY()       const { return velY;       }
float Entidad::getVida()       const { return vida;       }
float Entidad::getVidaMaxima() const { return vidaMaxima; }
float Entidad::getMasa()       const { return masa;       }
bool  Entidad::isActivo()      const { return activo;     }
