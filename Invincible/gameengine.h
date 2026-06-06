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

    // Constructor y destructor
    explicit GameEngine(const DifficultyConfig& config);
    ~GameEngine();

    // Ciclo principal llamado por QTimer
    void update(float dt);

    // Control de niveles
    void iniciarNivel(uint8_t nivel, QGraphicsScene* escena);
    void pausar();
    void reanudar();

    // Acceso al estado (solo lectura para la GUI)
    const GameState& getEstado() const;

    // Acceso al jugador (para input desde GameWidget)
    Jugador* getJugador() const;

    // Acceso a posiciones para sincronizar sprites
    const QList<Enemigo*>& getEnemigos() const;
    const QList<Portal*>&  getPortales() const;

    void aplicarAtaqueJugador();

private:

    // Entidades
    Jugador*         jugador;
    QList<Enemigo*>  enemigos;
    QList<Portal*>   portales;

    // Servicios y configuración
    PhysicsEngine    physics;
    GameState        estado;
    DifficultyConfig config;

    // Control de tiempo
    float    tiempoNivel2;
    float    tiempoSpawn;
    uint8_t  nivelActivo;
    bool     pausado;
    float tiempoVariantePortal;  // contador para spawn de proyectiles

    // Lógica por nivel
    void actualizarNivel1(float dt);
    void actualizarNivel2(float dt);

    // Colisiones
    void verificarColisiones();
    bool colisionAABB(float x1, float y1,
                      float x2, float y2,
                      float semi1, float semi2) const;

    // Limpieza y estado
    void limpiarInactivos();
    void actualizarGameState();
    void verificarCondicionFin();

    // ─── Control de Invasión Nivel 2 ───
    uint8_t totalVariantesSpawneadas;
    float tiempoSpawnVoladoras;
    float frecuenciaSpawnVoladoras;
    float tiempoFluctuacionPortales;
    int targetPortales;
    bool creciendoPortales;

    void gestionarPortalesEntorno(float dt);
    // Spawn de enemigos
    void spawnVariante();
    void spawnVoladora();

    QGraphicsScene* escena;
    Enemigo* misSeisVariantes[6];


};

#endif
