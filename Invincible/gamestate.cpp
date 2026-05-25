#include "gamestate.h"
#include <cstdint>

// ── Constructor ─────────────────────────────────────────────────
GameState::GameState()
    : nivelActual(1)
    , puntos(0)
    , tiempoRestante(0.0f)
    , vidaJugador(100.0f)
    , vidaMaxJugador(100.0f)
    , vidaLevy(150.0f)
    , vidaMaxLevy(150.0f)
    , hitStreak(0)
    , comboActivo(false)
    , estadoPartida(EstadoPartida::MENU)
{
}

// ── Setters ─────────────────────────────────────────────────────
void GameState::setNivel(uint8_t nivel)
{
    nivelActual = nivel;
}

void GameState::setPuntos(uint32_t p)
{
    puntos = p;
}

void GameState::sumarPuntos(uint32_t cantidad)
{
    puntos += cantidad;
}

void GameState::setTiempoRestante(float tiempo)
{
    if (tiempo < 0.0f) tiempo = 0.0f;
    tiempoRestante = tiempo;
}

void GameState::setVidaJugador(float vida, float vidaMax)
{
    vidaJugador    = vida;
    vidaMaxJugador = vidaMax;
}

void GameState::setVidaLevy(float vida, float vidaMax)
{
    vidaLevy    = vida;
    vidaMaxLevy = vidaMax;
}

void GameState::setHitStreak(uint8_t streak)
{
    hitStreak = streak;
}

void GameState::setComboActivo(bool activo)
{
    comboActivo = activo;
}

void GameState::setEstado(EstadoPartida estado)
{
    estadoPartida = estado;
}

// ── Getters ─────────────────────────────────────────────────────
uint8_t       GameState::getNivel()          const { return nivelActual;    }
uint32_t      GameState::getPuntos()         const { return puntos;         }
float         GameState::getTiempoRestante() const { return tiempoRestante; }
float         GameState::getVidaJugador()    const { return vidaJugador;    }
float         GameState::getVidaMaxJugador() const { return vidaMaxJugador; }
float         GameState::getVidaLevy()       const { return vidaLevy;       }
float         GameState::getVidaMaxLevy()    const { return vidaMaxLevy;    }
uint8_t       GameState::getHitStreak()      const { return hitStreak;      }
bool          GameState::isComboActivo()     const { return comboActivo;    }

GameState::EstadoPartida GameState::getEstado() const
{
    return estadoPartida;
}
