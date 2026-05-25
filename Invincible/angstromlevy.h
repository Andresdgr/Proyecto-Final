#ifndef ANGSTROMLEVY_H
#define ANGSTROMLEVY_H

#include "enemigo.h"
#include <deque>
#include <cstdint>

class Jugador;

class AngstromLevy : public Enemigo {

public:

    // ── Enum de estados — tipo subyacente uint8_t ────────────
    enum class EstadoIA : uint8_t {
        AGRESIVO   = 0,
        EVASIVO    = 1,
        DEFENSIVO  = 2
    };

    // ── Constantes de diseño del agente ─────────────────────
    static constexpr float   UMBRAL_DISTANCIA_EVASION = 100.0f;
    static constexpr float   UMBRAL_VIDA_DEFENSIVO    = 0.30f;
    static constexpr float   UMBRAL_VIDA_AGRESIVO     = 0.70f;
    static constexpr float   AGRESIVIDAD_MAX          = 2.0f;
    static constexpr float   FRECUENCIA_UMBRAL        = 0.65f;
    static constexpr float   RADIO_PORTAL_BASE        = 220.0f;
    static constexpr float   RADIO_PORTAL_AJUSTADO    = 180.0f;
    static constexpr uint8_t CAPACIDAD_HISTORIAL      = 10;
    static constexpr uint8_t INTERVALO_APRENDIZAJE    = 5;

    // ── Constructor y destructor ─────────────────────────────
    AngstromLevy(float x, float y);
    virtual ~AngstromLevy();

    // ── Permite que ambas versiones de update coexistan ─────
    using Enemigo::update;

    // ── Implementación de métodos abstractos ─────────────────
    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;

    // ── Getter del estado actual (para DebugOverlay C3) ─────
    EstadoIA getEstadoActual() const;

private:

    // ── Estado cognitivo ─────────────────────────────────────
    EstadoIA                estadoActual;
    std::deque<int8_t>      historialDirecciones;
    float                   agresividad;
    float                   radioPortal;
    uint16_t                contadorInteracciones;

    // ── Datos percibidos (snapshot del entorno) ──────────────
    float    distanciaAlJugador;
    float    vidaJugadorPercibida;
    float    xJugadorPercibida;
    int8_t   direccionPercibida;
    bool     jugadorCercano;

    // ── Los 4 componentes del agente (privados) ──────────────
    void percibir(const Jugador& jugador);
    void razonar();
    void actuar(float dt);
    void aprender();

    // ── Acciones disponibles ─────────────────────────────────
    void teleportarse();
    void esperarYDañar();

    // ── Helpers del aprendizaje ──────────────────────────────
    float calcularFrecuenciaDerecha() const;
    void  ajustarParametros();

};

#endif
