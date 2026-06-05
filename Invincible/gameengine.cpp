#include "gameengine.h"
#include "angstromlevy.h"
#include "variante.h"
#include "varianteportal.h"
#include <stdexcept>
#include <cstdlib>
#include <cmath>

// ── Constructor ─────────────────────────────────────────────────
GameEngine::GameEngine(const DifficultyConfig& config)
    : jugador(nullptr)
    , physics(config)
    , config(config)
    , tiempoNivel2(60.0f)
    , tiempoSpawn(0.0f)
    , tiempoProyectil(0.0f)
    , nivelActivo(0)
    , pausado(false)
{
}

// ── Destructor ───────────────────────────────────────────────────
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
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : enemigos) delete e;
    enemigos.clear();

    for (Portal* p : portales) delete p;
    portales.clear();

    jugador = new Jugador(400.0f, 300.0f);

    nivelActivo = nivel;
    estado.setNivel(nivel);
    estado.setPuntos(0);
    estado.setEstado(GameState::EstadoPartida::JUGANDO);

    if (nivel == 1) {
        AngstromLevy* levy = new AngstromLevy(600.0f, 300.0f);
        enemigos.append(levy);

        Portal* portal = new Portal(400.0f, 250.0f,
                                    config.amplitudMAS,
                                    config.omegaMAS);
        portales.append(portal);

    } else if (nivel == 2) {
        tiempoNivel2 = 60.0f;
        tiempoSpawn  = 0.0f;
        estado.setTiempoRestante(tiempoNivel2);

        // Portal orbital para Nivel 2
        Portal* portal = new Portal(400.0f, 250.0f,
                                    config.radioPortalNivel2,
                                    config.omegaPortal,
                                    0.0f);
        portales.append(portal);

    } else {
        throw std::invalid_argument(
            "GameEngine::iniciarNivel — nivel debe ser 1 o 2");
    }
}

// ── update ──────────────────────────────────────────────────────
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

// ── Nivel 1 ──────────────────────────────────────────────────────
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

// ── Nivel 2 ──────────────────────────────────────────────────────
void GameEngine::actualizarNivel2(float dt)
{
    tiempoNivel2 -= dt;
    if (tiempoNivel2 < 0.0f) tiempoNivel2 = 0.0f;
    estado.setTiempoRestante(tiempoNivel2);

    // Spawn variantes — máximo 6
    if (enemigos.size() < 6) {
        tiempoSpawn += dt;
        if (tiempoSpawn >= config.frecuenciaSpawn) {
            spawnClon();
            tiempoSpawn = 0.0f;
        }
    }

    // Spawn proyectil desde portal cada 3 segundos
    tiempoProyectil += dt;
    if (tiempoProyectil >= 3.0f) {
        spawnProyectil();
        tiempoProyectil = 0.0f;
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
// ── aplicarAtaqueJugador ─────────────────────────────────────────
void GameEngine::aplicarAtaqueJugador()
{
    if (!jugador || !jugador->isActivo()) return;
    if (jugador->getDanioActual() <= 0.0f) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    float jIzq    = xJ - 20.0f;
    float jDer    = xJ + 80.0f;
    float jArriba = yJ - 10.0f;
    float jAbajo  = yJ + 90.0f;

    for (Enemigo* e : enemigos) {
        if (!e->isActivo()) continue;

        float eIzq    = e->getX();
        float eDer    = e->getX() + 70.0f;
        float eArriba = e->getY();
        float eAbajo  = e->getY() + 90.0f;

        bool colision = jDer    > eIzq    &&
                        jIzq    < eDer    &&
                        jAbajo  > eArriba &&
                        jArriba < eAbajo;

        if (colision) {
            e->recibirDanio(jugador->getDanioActual());
            estado.sumarPuntos(10);

            // Impulso parabólico solo para AngstromLevy
            AngstromLevy* levy = dynamic_cast<AngstromLevy*>(e);
            if (levy) {
                float dirX = (e->getX() > xJ) ? 1.0f : -1.0f;
                levy->recibirImpacto(dirX);
            }
        }
    }
}

// ── verificarColisiones ──────────────────────────────────────────
void GameEngine::verificarColisiones()
{
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    float jIzq    = xJ;
    float jDer    = xJ + 60.0f;
    float jArriba = yJ;
    float jAbajo  = yJ + 80.0f;

    // Enemigos dañan al jugador por contacto
    for (Enemigo* e : enemigos) {
        if (!e->isActivo()) continue;

        float eIzq    = e->getX();
        float eDer    = e->getX() + 70.0f;
        float eArriba = e->getY();
        float eAbajo  = e->getY() + 90.0f;

        bool colision = jDer    > eIzq    &&
                        jIzq    < eDer    &&
                        jAbajo  > eArriba &&
                        jArriba < eAbajo;

        if (colision) {
            jugador->recibirDanio(e->getDanio());
        }
    }

    // Portal — daño ambiental continuo
    for (Portal* p : portales) {
        if (!p->isActivo()) continue;

        if (p->jugadorTocaBorde(xJ, yJ, 60.0f, 80.0f)) {
            jugador->recibirDanioAmbiental(config.danioPortal * 0.016f);
        }
    }
}

// ── colisionAABB ─────────────────────────────────────────────────
bool GameEngine::colisionAABB(float x1, float y1,
                              float x2, float y2,
                              float semi1, float semi2) const
{
    return (std::abs(x1 - x2) < (semi1 + semi2)) &&
           (std::abs(y1 - y2) < (semi1 + semi2));
}

// ── limpiarInactivos ─────────────────────────────────────────────
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

// ── actualizarGameState ──────────────────────────────────────────
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

// ── verificarCondicionFin ────────────────────────────────────────
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

// ── spawnClon ────────────────────────────────────────────────────
void GameEngine::spawnClon()
{
    if (!jugador || !jugador->isActivo()) return;

    float x, y;
    int borde = rand() % 4;

    switch (borde) {
    case 0: x = 0.0f;   y = static_cast<float>(rand() % 480); break;
    case 1: x = 750.0f; y = static_cast<float>(rand() % 480); break;
    case 2: x = static_cast<float>(rand() % 750); y = 0.0f;   break;
    case 3: x = static_cast<float>(rand() % 750); y = 480.0f; break;
    default: x = 0.0f; y = 0.0f;
    }

    Variante* clon = new Variante(x, y,
                                  50.0f,
                                  40.0f,
                                  config.danioLevy * 0.5f,
                                  50);
    enemigos.append(clon);
}

// ── pausar / reanudar ────────────────────────────────────────────
void GameEngine::pausar()   { pausado = true;  }
void GameEngine::reanudar() { pausado = false; }

// ── Getters ──────────────────────────────────────────────────────
const GameState&       GameEngine::getEstado()   const { return estado;   }
Jugador*               GameEngine::getJugador()  const { return jugador;  }
const QList<Enemigo*>& GameEngine::getEnemigos() const { return enemigos; }
const QList<Portal*>&  GameEngine::getPortales() const { return portales; }


void GameEngine::spawnProyectil()
{
    if (!jugador || !jugador->isActivo()) return;
    if (portales.isEmpty()) return;

    Portal* portal = portales[0];
    float xP = portal->getX();
    float yP = portal->getY();

    float dx   = jugador->getX() - xP;
    float dy   = jugador->getY() - yP;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 1.0f) return;

    VariantePortal* proy = new VariantePortal(
        xP, yP,
        1.0f,
        1.0f,
        config.danioLevy * 0.3f,
        0);

    float vel = 300.0f;
    proy->setVelocidad((dx / dist) * vel, (dy / dist) * vel);

    enemigos.append(proy);
}
