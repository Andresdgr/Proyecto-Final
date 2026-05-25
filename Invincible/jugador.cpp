#include "jugador.h"

// ── Constructor ─────────────────────────────────────────────────
Jugador::Jugador(float x, float y)
    : Entidad(x, y, 100.0f, 75.0f)
    , velBase(MULTIPLICADOR_VELO * 3.0f)
    , cargando(false)
    , tiempoCarga(0.0f)
    , hitStreak(0)
    , comboActivo(false)
    , invulnerable(false)
    , tiempoInvulnerable(0.0f)
    , danioActual(0.0f)
{
}

// ── Destructor ──────────────────────────────────────────────────
Jugador::~Jugador()
{
}

// ── update ──────────────────────────────────────────────────────
void Jugador::update(float dt)
{
    // 1. Actualizar timer de carga
    if (cargando) {
        tiempoCarga += dt;
        velX = 0.0f; // vulnerable durante la carga
    }

    // 2. Actualizar timer de invulnerabilidad
    if (invulnerable) {
        tiempoInvulnerable -= dt;
        if (tiempoInvulnerable <= 0.0f) {
            tiempoInvulnerable = 0.0f;
            invulnerable       = false;
        }
    }

    // 3. Aplicar movimiento
    x += velX * dt;
}

// ── moverX ──────────────────────────────────────────────────────
void Jugador::moverX(float direccion)
{
    if (cargando) return; // no puede moverse mientras carga
    velX = direccion * velBase;
}

// ── detener ─────────────────────────────────────────────────────
void Jugador::detener()
{
    if (cargando) return;
    velX = 0.0f;
}

// ── iniciarCarga ────────────────────────────────────────────────
void Jugador::iniciarCarga()
{
    if (cargando) return; // evita reiniciar si ya está cargando
    cargando    = true;
    tiempoCarga = 0.0f;
    velX        = 0.0f;
}

// ── soltarCarga ─────────────────────────────────────────────────
void Jugador::soltarCarga()
{
    // Solo marca que dejó de cargar.
    // El daño se calcula cuando se llame atacar().
    cargando = false;
}

// ── atacar ──────────────────────────────────────────────────────
void Jugador::atacar()
{
    float danioBase = 20.0f;

    // Determinar si el golpe es poderoso
    bool golpePotente = (tiempoCarga >= UMBRAL_CARGA) || comboActivo;

    if (golpePotente) {
        danioActual = danioBase * FACTOR_DAÑO_CARGA;
    } else {
        danioActual = danioBase;
    }

    // Gestionar combo
    if (golpePotente && comboActivo) {
        // El combo se consumió — resetear
        hitStreak   = 0;
        comboActivo = false;
    } else {
        hitStreak++;
        if (hitStreak >= GOLPES_COMBO) {
            comboActivo = true;
        }
    }

    // Resetear carga
    cargando    = false;
    tiempoCarga = 0.0f;
}

// ── recibirDanio ────────────────────────────────────────────────
void Jugador::recibirDanio(float cantidad)
{
    // Si está invulnerable, ignora el daño completamente
    if (invulnerable) return;

    // Llamar la lógica base — resta vida y marca inactivo si llega a 0
    Entidad::recibirDanio(cantidad);

    // Comportamiento adicional exclusivo del jugador
    hitStreak          = 0;
    comboActivo        = false;
    invulnerable       = true;
    tiempoInvulnerable = DURACION_INVUL;
}

// ── Getters ─────────────────────────────────────────────────────
float   Jugador::getDanioActual()  const { return danioActual;        }
uint8_t Jugador::getHitStreak()    const { return hitStreak;          }
bool    Jugador::isInvulnerable()  const { return invulnerable;       }
bool    Jugador::isCargando()      const { return cargando;           }
bool    Jugador::isComboActivo()   const { return comboActivo;        }
