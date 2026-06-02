#ifndef JUGADOR_H
#define JUGADOR_H

#include "entidad.h"
#include <cstdint>

class Jugador : public Entidad {

public:

    // ── Constantes de diseño del personaje ──────────────────
    static constexpr float   MULTIPLICADOR_VELO = 1.4f;
    static constexpr float   UMBRAL_CARGA       = 0.4f;
    static constexpr float   FACTOR_DAÑO_CARGA  = 2.0f;
    static constexpr float   DURACION_INVUL      = 0.8f;
    static constexpr uint8_t GOLPES_COMBO        = 3;

    // ── Constructor y destructor ─────────────────────────────
    Jugador(float x, float y);

    virtual ~Jugador();

    // ── Ciclo de vida por tick ───────────────────────────────
    void update(float dt) override;

    // ── Instrucciones de movimiento (llamadas desde GUI) ─────
    void moverX(float direccion);
    void moverY(float direccion);
    void detener();

    // ── Instrucciones de ataque ──────────────────────────────
    void iniciarCarga();
    void soltarCarga();
    void atacar();
    void resetDanio();

    // ── Sobreescritura con lógica de invulnerabilidad ────────
    void recibirDanio(float cantidad) override;

    // ── Getters propios ──────────────────────────────────────
    float   getDanioActual()   const;
    uint8_t getHitStreak()     const;
    bool    isInvulnerable()   const;
    bool    isCargando()       const;
    bool    isComboActivo()    const;
    //--------------danño por el portal-----------------
    void recibirDanioAmbiental(float cantidad);

private:

    // ── Característica: veloz ────────────────────────────────
    float velBase;

    // ── Característica: fortachón ────────────────────────────
    bool  cargando;
    float tiempoCarga;

    // ── Sistema de combo ─────────────────────────────────────
    uint8_t hitStreak;
    bool    comboActivo;

    // ── Invulnerabilidad post-golpe ──────────────────────────
    bool  invulnerable;
    float tiempoInvulnerable;

    // ── Resultado del último ataque ──────────────────────────
    float danioActual;

};

#endif
