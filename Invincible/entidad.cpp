#include "entidad.h"

// ── Constructor ─────────────────────────────────────────────────
Entidad::Entidad(float x, float y, float vida, float masa, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent) // Inicializa la clase base de Qt
    , x(x)
    , y(y)
    , velX(0.0f)
    , velY(0.0f)
    , vida(vida)
    , vidaMaxima(vida)
    , masa(masa)
    , activo(true)
    , tiempoRecuperacionGolpe(0.0f)
{
    // Sincronizar las coordenadas iniciales matemáticas con las gráficas
    setPos(x, y);
}

// ── Destructor ──────────────────────────────────────────────────
Entidad::~Entidad()
{
}

// ── recibirDanio ────────────────────────────────────────────────
void Entidad::recibirDanio(float cantidad)
{
    if (!activo) return;

    vida -= cantidad;
    tiempoRecuperacionGolpe = 0.5f;

    if (vida <= 0.0f) {
        vida   = 0.0f;
        activo = false;

        // Beneficio de Qt: oculta el sprite de la escena instantáneamente
        hide();
    }
}

// ── Setters ─────────────────────────────────────────────────────
void Entidad::setPosicion(float nuevoX, float nuevoY)
{
    x = nuevoX;
    y = nuevoY;

    //cada que se cambia  (x, y), actualiza okla vista en pantalla
    setPos(x, y);
}

void Entidad::setVelocidad(float vX, float vY)
{
    velX = vX;
    velY = vY;
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

void Entidad::limitarBordes(float limiteX, float limiteY, float anchoSprite, float altoSprite) {
    if (x < 0.0f) x = 0.0f;
    if (x > limiteX - anchoSprite) x = limiteX - anchoSprite;

    if (y < 0.0f) y = 0.0f;
    if (y > limiteY - altoSprite) y = limiteY - altoSprite;
}

float Entidad::getTiempoRecuperacionGolpe() const {
    return tiempoRecuperacionGolpe;
}

void Entidad::reducirTiempoRecuperacion(float dt) {
    if (tiempoRecuperacionGolpe > 0.0f) {
        tiempoRecuperacionGolpe -= dt;
        if (tiempoRecuperacionGolpe < 0.0f) tiempoRecuperacionGolpe = 0.0f;
    }
}

void Entidad::setVida(float nuevaVida)
{
    if (nuevaVida <= 0.0f) return;
    vida       = nuevaVida;
    vidaMaxima = nuevaVida;
}
