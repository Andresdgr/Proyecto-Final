#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "jugador.h"
#include "enemigo.h"
#include "portal.h"
#include "varianteportal.h"
#include "physicsengine.h"
#include "gamestate.h"
#include "difficultyconfig.h"
#include <QList>
#include <cstdint>

class GameEngine {

public:
    explicit GameEngine(const DifficultyConfig& config);
    ~GameEngine();

    void update(float dt);
    void iniciarNivel(uint8_t nivel);
    void pausar();
    void reanudar();

    const GameState&       getEstado()   const;
    Jugador*               getJugador()  const;
    const QList<Enemigo*>& getEnemigos() const;
    const QList<Portal*>&  getPortales() const;

    void aplicarAtaqueJugador();

private:
    // ── Entidades ────────────────────────────────────────────
    Jugador*         jugador;
    QList<Enemigo*>  enemigos;
    QList<Portal*>   portales;

    // ── Servicios y configuración ────────────────────────────
    PhysicsEngine    physics;
    GameState        estado;
    DifficultyConfig config;

    // ── Control de tiempo ────────────────────────────────────
    float    tiempoNivel2;
    float    tiempoSpawn;
    float    tiempoSpawnVoladoras;
    float    frecuenciaSpawnVoladoras;
    float    tiempoFluctuacionPortales;
    float    tiempoProyectil;
    uint8_t  nivelActivo;
    bool     pausado;
    int      targetPortales;
    bool     creciendoPortales;
    int      totalVariantesSpawneadas;

    // ── Lógica por nivel ─────────────────────────────────────
    void actualizarNivel1(float dt);
    void actualizarNivel2(float dt);

    // ── Colisiones ───────────────────────────────────────────
    void verificarColisiones();
    bool colisionAABB(float x1, float y1,
                      float x2, float y2,
                      float semi1, float semi2) const;

    // ── Limpieza y estado ────────────────────────────────────
    void limpiarInactivos();
    void actualizarGameState();
    void verificarCondicionFin();

    // ── Spawn y gestión de enemigos ──────────────────────────
    void spawnVariante();
    void spawnVoladora();
    void gestionarPortalesEntorno(float dt);
};

#endif
