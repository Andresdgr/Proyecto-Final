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
    setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Invincible_85x85.png"));
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
        velY = 0.0f;
        velX = 0.0f; // vulnerable durante la carga
    }

    // 2. Timer de invulnerabilidad
    if (invulnerable) {
        tiempoInvulnerable -= dt;
        if (tiempoInvulnerable <= 0.0f) {
            tiempoInvulnerable = 0.0f;
            invulnerable       = false;
        }
    }

    // 3. Aplicar movimiento
    x += velX * dt;
    y += velY * dt;
    // 4. Actualizar la posición visual en la escena de Qt
    setPos(x, y);
}

// ── moverX ──────────────────────────────────────────────────────
void Jugador::moverX(float direccion)
{
    if (cargando) return; // no puede moverse mientras carga
    velX = direccion * velBase;
}

// ── detener ─────────────────────────────────────────────────────
void Jugador::detenerX()
{
    if (cargando) return;
    velX = 0.0f;
}
// ── MOVIMIENTO VERTICAL ──────────────────────
void Jugador::moverY(float direccion)
{
    if (cargando) return; // No puede moverse mientras carga el ataque
    velY = direccion * velBase;
}
void Jugador::detenerY()
{
    if (cargando) return;
    velY = 0.0f;
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

void Jugador::detenerAtaque() {
    // 1. Restablecer el daño actual a cero para que dejen de contar las colisiones de ataque
    this->danioActual = 0.0f;

    // this->atacando = false;

    // Cambio de sprites, restablece el pixmap
    // this->setPixmap(QPixmap(".../Sprites/Invincible.png"));
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
