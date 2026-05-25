#include "gameengine.h"
#include "angstromlevy.h"
#include <stdexcept>

// ── Constructor ─────────────────────────────────────────────────
GameEngine::GameEngine(const DifficultyConfig& config)
    : jugador(nullptr)
    , physics(config)
    , config(config)
    , tiempoNivel2(60.0f)
    , tiempoSpawn(0.0f)
    , nivelActivo(0)
    , pausado(false)
{
}

// ── Destructor — libera toda la memoria dinámica ─────────────────
GameEngine::~GameEngine()
{
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : enemigos) delete e;
    enemigos.clear();

    for (Portal* p : portales) delete p;
    portales.clear();
}

// ── iniciarNivel ────────────────────────────────────────────────
void GameEngine::iniciarNivel(uint8_t nivel)
{
    // Limpiar estado anterior
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : enemigos) delete e;
    enemigos.clear();

    for (Portal* p : portales) delete p;
    portales.clear();

    // Crear jugador centrado en la pantalla
    jugador = new Jugador(400.0f, 300.0f);

    nivelActivo = nivel;
    estado.setNivel(nivel);
    estado.setEstado(GameState::EstadoPartida::JUGANDO);

    if (nivel == 1) {
        // Crear jefe Angstrom Levy
        AngstromLevy* levy = new AngstromLevy(600.0f, 300.0f);
        enemigos.append(levy);

        // Crear portal del nivel 1
        Portal* portal = new Portal(500.0f, 200.0f,
                                    config.amplitudMAS,
                                    config.omegaMAS);
        portales.append(portal);

    } else if (nivel == 2) {
        tiempoNivel2 = 60.0f;
        tiempoSpawn  = 0.0f;
        estado.setTiempoRestante(tiempoNivel2);

    } else {
        throw std::invalid_argument(
            "GameEngine::iniciarNivel — nivel debe ser 1 o 2");
    }
}

// ── update — orquestador principal ──────────────────────────────
void GameEngine::update(float dt)
{
    if (pausado) return;

    if (estado.getEstado() != GameState::EstadoPartida::JUGANDO) return;

    physics.setDt(dt);

    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
    }

    if      (nivelActivo == 1) actualizarNivel1(dt);
    else if (nivelActivo == 2) actualizarNivel2(dt);

    verificarColisiones();
    limpiarInactivos();
    actualizarGameState();
    verificarCondicionFin();
}

// ── Nivel 1 — jefe + portales MAS ───────────────────────────────
void GameEngine::actualizarNivel1(float dt)
{
    for (Enemigo* e : enemigos) {
        if (e->isActivo() && jugador) {
            e->update(dt, *jugador);
        }
    }

    for (Portal* p : portales) {
        if (p->isActivo()) {
            p->updateConPhysics(dt, physics);
        }
    }
}

// ── Nivel 2 — horda + timer + spawn ─────────────────────────────
void GameEngine::actualizarNivel2(float dt)
{
    tiempoNivel2 -= dt;
    if (tiempoNivel2 < 0.0f) tiempoNivel2 = 0.0f;
    estado.setTiempoRestante(tiempoNivel2);

    tiempoSpawn += dt;
    if (tiempoSpawn >= config.frecuenciaSpawn) {
        spawnClon();
        tiempoSpawn = 0.0f;
    }

    for (Enemigo* e : enemigos) {
        if (e->isActivo() && jugador) {
            e->update(dt, *jugador);
        }
    }

    for (Portal* p : portales) {
        if (p->isActivo()) {
            p->updateConPhysics(dt, physics);
        }
    }
}

// ── Colisiones AABB ──────────────────────────────────────────────
void GameEngine::verificarColisiones()
{
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    for (Enemigo* e : enemigos) {
        if (!e->isActivo()) continue;

        if (colisionAABB(xJ, yJ, e->getX(), e->getY(), 30.0f, 30.0f)) {
            jugador->recibirDanio(e->getDanio());

            if (jugador->getDanioActual() > 0.0f) {
                e->recibirDanio(jugador->getDanioActual());
                estado.sumarPuntos(10);
            }
        }
    }

    for (Portal* p : portales) {
        if (!p->isActivo()) continue;

        if (p->jugadorEnZonaDanio(xJ, yJ)) {
            jugador->recibirDanio(config.danioPortal * 0.016f);
        }
    }

    if (nivelActivo == 2) {
        for (Enemigo* e : enemigos) {
            if (!e->isActivo()) continue;

            float xE = e->getX();
            if (xE < 0.0f || xE > 800.0f) {
                physics.reboteBorde(e->getVelX(), 0.7f);
            }
        }
    }
}

// ── AABB helper ──────────────────────────────────────────────────
bool GameEngine::colisionAABB(float x1, float y1,
                              float x2, float y2,
                              float semi1, float semi2) const
{
    return (std::abs(x1 - x2) < (semi1 + semi2)) &&
           (std::abs(y1 - y2) < (semi1 + semi2));
}

// ── Limpiar entidades inactivas ──────────────────────────────────
void GameEngine::limpiarInactivos()
{
    for (int i = enemigos.size() - 1; i >= 0; i--) {
        if (!enemigos[i]->isActivo()) {
            estado.sumarPuntos(enemigos[i]->getPuntosAlDerrotar());
            delete enemigos[i];
            enemigos.removeAt(i);
        }
    }

    for (int i = portales.size() - 1; i >= 0; i--) {
        if (!portales[i]->isActivo()) {
            delete portales[i];
            portales.removeAt(i);
        }
    }
}

// ── Actualizar GameState ─────────────────────────────────────────
void GameEngine::actualizarGameState()
{
    if (jugador) {
        estado.setVidaJugador(jugador->getVida(),
                              jugador->getVidaMaxima());
        estado.setHitStreak(jugador->getHitStreak());
        estado.setComboActivo(jugador->isComboActivo());
    }

    if (nivelActivo == 1 && !enemigos.isEmpty()) {
        estado.setVidaLevy(enemigos[0]->getVida(),
                           enemigos[0]->getVidaMaxima());
    }
}

// ── Verificar condición de fin ───────────────────────────────────
void GameEngine::verificarCondicionFin()
{
    if (!jugador || !jugador->isActivo()) {
        estado.setEstado(GameState::EstadoPartida::DERROTA);
        return;
    }

    if (nivelActivo == 1 && enemigos.isEmpty()) {
        estado.setEstado(GameState::EstadoPartida::VICTORIA);
        return;
    }

    if (nivelActivo == 2 && tiempoNivel2 <= 0.0f) {
        estado.setEstado(GameState::EstadoPartida::VICTORIA);
        return;
    }
}

// ── Spawn de clon — completar en merge con rama nivel-2 ─────────
void GameEngine::spawnClon()
{
}

// ── Pausar y reanudar ────────────────────────────────────────────
void GameEngine::pausar()   { pausado = true;  }
void GameEngine::reanudar() { pausado = false; }

// ── Getters ──────────────────────────────────────────────────────
const GameState& GameEngine::getEstado()  const { return estado;  }
Jugador*         GameEngine::getJugador() const { return jugador; }
