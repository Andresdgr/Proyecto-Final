#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "nivel.h"
#include "gamestate.h"
#include "difficultyconfig.h"
#include "physicsengine.h"
#include <QGraphicsScene>
#include <cstdint>

class GameEngine {

public:

    // ── Constructor y destructor ─────────────────────────────
    explicit GameEngine(const DifficultyConfig& config);
    ~GameEngine();

    // ── Ciclo principal llamado por QTimer ───────────────────
    void update(float dt);

    // ── Control de niveles ───────────────────────────────────
    void iniciarNivel(uint8_t nivel, QGraphicsScene* escenaCompartida);
    void pausar();
    void reanudar();

    // ── Acceso al estado (solo lectura para la GUI) ──────────
    const GameState& getEstado() const;

    // ── Acceso al jugador (para input desde GameWidget) ──────
    Jugador* getJugador() const;

private:
    // Puntero polimórfico a la clase base del nivel activo
    // Reemplaza las listas de enemigos, portales y el puntero directo al jugador.
    Nivel* nivelActual;

    // ── Servicios y configuración ────────────────────────────
    PhysicsEngine    physics;
    GameState        estado;
    DifficultyConfig config;

    // ── Control de estado interno ────────────────────────────
    uint8_t  idNivelActivo;
    bool     pausado;

    // ── Limpieza y sincronización global ─────────────────────
    void actualizarGameState();
    void verificarCondicionFin();
};

#endif // GAMEENGINE_H
