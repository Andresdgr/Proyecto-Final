#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cstdint>

class GameState {

public:

    // ── Enum del estado de la partida ────────────────────────
    enum class EstadoPartida : uint8_t {
        MENU     = 0,
        JUGANDO  = 1,
        PAUSA    = 2,
        VICTORIA = 3,
        DERROTA  = 4
    };

    // ── Constructor ──────────────────────────────────────────
    GameState();

    // ── Métodos de escritura (llamados por GameEngine) ───────
    void setNivel(uint8_t nivel);
    void setPuntos(uint32_t puntos);
    void sumarPuntos(uint32_t cantidad);
    void setTiempoRestante(float tiempo);
    void setVidaJugador(float vida, float vidaMax);
    void setVidaLevy(float vida, float vidaMax);
    void setHitStreak(uint8_t streak);
    void setComboActivo(bool activo);
    void setEstado(EstadoPartida estado);

    // ── Getters (llamados por la capa GUI) ───────────────────
    uint8_t        getNivel()           const;
    uint32_t       getPuntos()          const;
    float          getTiempoRestante()  const;
    float          getVidaJugador()     const;
    float          getVidaMaxJugador()  const;
    float          getVidaLevy()        const;
    float          getVidaMaxLevy()     const;
    uint8_t        getHitStreak()       const;
    bool           isComboActivo()      const;
    EstadoPartida  getEstado()          const;

private:

    uint8_t       nivelActual;
    uint32_t      puntos;
    float         tiempoRestante;

    float         vidaJugador;
    float         vidaMaxJugador;

    float         vidaLevy;
    float         vidaMaxLevy;

    uint8_t       hitStreak;
    bool          comboActivo;

    EstadoPartida estadoPartida;

};

#endif
