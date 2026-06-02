#include "portal.h"
#include <cmath>

// ── Constructor Nivel 1 — MAS ────────────────────────────────────
Portal::Portal(float x, float y, float amplitud, float omegaMAS)
    : Entidad(x, y, 1.0f, 0.0f)
    , tiempoVida(0.0f)
    , amplitud(amplitud)
    , omegaMAS(omegaMAS)
    , radioActual(amplitud)
    , cx(x)
    , cy(y)
    , radioOrbital(0.0f)
    , omegaOrbital(0.0f)
    , fase(0.0f)
    , esPuntoSpawn(false)
{
}

// ── Constructor Nivel 2 — Orbital ────────────────────────────────
Portal::Portal(float cx, float cy, float radioOrbital,
               float omegaOrbital, float fase)
    : Entidad(cx + radioOrbital * std::cos(fase),
              cy + radioOrbital * std::sin(fase),
              1.0f, 0.0f)
    , tiempoVida(0.0f)
    , amplitud(0.0f)
    , omegaMAS(0.0f)
    , radioActual(30.0f)
    , cx(cx)
    , cy(cy)
    , radioOrbital(radioOrbital)
    , omegaOrbital(omegaOrbital)
    , fase(fase)
    , esPuntoSpawn(true)
{
    setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Portal.png"));
}

// ── Destructor ───────────────────────────────────────────────────
Portal::~Portal()
{
}

// ── update básico — acumula tiempo ───────────────────────────────
void Portal::update(float dt)
{
    if (!activo) return;
    tiempoVida += dt;
}

// ── update con physics — calcula posición y radio ────────────────
void Portal::updateConPhysics(float dt, const PhysicsEngine& physics)
{
    if (!activo) return;

    tiempoVida += dt;

    if (!esPuntoSpawn) {
        // Nivel 1 — MAS: actualizar radio de zona de daño
        radioActual = physics.MAS(amplitud, omegaMAS, tiempoVida);

        // Asegurar que el radio no sea negativo
        if (radioActual < 0.0f) radioActual = -radioActual;

    } else {
        // Nivel 2 — Orbital adaptado a la elipse de la pantalla (800x600)
        // Multiplicamos el eje X por (400/300) para que llegue a los bordes laterales
        float rx = radioOrbital * (350.0f / 300.0f);
        float ry = radioOrbital * (200.0f / 300.0f);

        float xFisica = cx + rx * std::cos(omegaOrbital * tiempoVida + fase);
        float yFisica = cy + ry * std::sin(omegaOrbital * tiempoVida + fase);

        x = xFisica;
        y = yFisica;

        setPosicion(x, y); // Sincroniza con Qt
    }
}

// ── jugadorEnZonaDanio ───────────────────────────────────────────
bool Portal::jugadorEnZonaDanio(float xJ, float yJ) const
{
    if (!activo) return false;

    float dx       = xJ - x;
    float dy       = yJ - y;
    float distancia = std::sqrt(dx * dx + dy * dy);

    return distancia < radioActual;
}

// ── Getters ──────────────────────────────────────────────────────
float Portal::getRadioActual() const { return radioActual; }
bool  Portal::isPuntoSpawn()   const { return esPuntoSpawn; }
