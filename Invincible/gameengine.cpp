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
    , tiempoSpawnVoladoras(0.0f)
    , frecuenciaSpawnVoladoras(1.0f)
    , tiempoFluctuacionPortales(0.0f)
    , tiempoProyectil(0.0f)
    , nivelActivo(0)
    , pausado(false)
    , targetPortales(0)
    , creciendoPortales(true)
    , totalVariantesSpawneadas(0)
{
}

// ── Destructor ───────────────────────────────────────────────────
GameEngine::~GameEngine()
{
    delete jugador;
    jugador = nullptr;
    for (Enemigo* e : std::as_const(enemigos)) delete e;
    enemigos.clear();
    for (Portal* p : std::as_const(portales)) delete p;
    portales.clear();
}

// ── iniciarNivel ─────────────────────────────────────────────────
void GameEngine::iniciarNivel(uint8_t nivel)
{
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : std::as_const(enemigos)) delete e;
    enemigos.clear();

    for (Portal* p : std::as_const(portales)) delete p;
    portales.clear();

    totalVariantesSpawneadas  = 0;
    targetPortales            = 0;
    creciendoPortales         = true;
    tiempoFluctuacionPortales = 0.0f;
    tiempoSpawnVoladoras      = 0.0f;
    frecuenciaSpawnVoladoras  = 1.0f;

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
        tiempoNivel2 = 90.0f;
        tiempoSpawn  = 0.0f;
        estado.setTiempoRestante(tiempoNivel2);

        // 4 variantes de combate + 2 voladoras — lista polimórfica
        for (int i = 0; i < 6; ++i) {
            Enemigo* v = nullptr;
            if (i == 2 || i == 5) {
                v = new VariantePortal(-1000.0f, -1000.0f,
                                       300.0f, 75.0f,
                                       config.danioLevy * 0.3f, 150);
            } else {
                v = new Variante(-1000.0f, -1000.0f,
                                 500.0f, 75.0f,
                                 config.danioLevy * 0.5f, 100);
            }
            v->hide();
            enemigos.append(v);
        }

        // Portal orbital inicial
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

// ── update ───────────────────────────────────────────────────────
void GameEngine::update(float dt)
{
    if (pausado) return;
    if (estado.getEstado() != GameState::EstadoPartida::JUGANDO) return;

    physics.setDt(dt);

    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
        // Agente inteligente percibe posición del jugador
        VariantePortal::percibir(jugador->getX(), jugador->getY());
    }

    if      (nivelActivo == 1) actualizarNivel1(dt);
    else if (nivelActivo == 2) actualizarNivel2(dt);

    verificarColisiones();
    limpiarInactivos();
    actualizarGameState();
    verificarCondicionFin();
}

// ── actualizarNivel1 ─────────────────────────────────────────────
void GameEngine::actualizarNivel1(float dt)
{
    for (Enemigo* e : std::as_const(enemigos)) {
        if (e->isActivo() && jugador) {
            e->update(dt, *jugador);
        }
    }

    for (Portal* p : std::as_const(portales)) {
        if (p->isActivo()) {
            p->updateConPhysics(dt, physics);
        }
    }
}

// ── actualizarNivel2 ─────────────────────────────────────────────
void GameEngine::actualizarNivel2(float dt)
{
    tiempoNivel2 -= dt;
    if (tiempoNivel2 < 0.0f) tiempoNivel2 = 0.0f;
    estado.setTiempoRestante(tiempoNivel2);

    // 1. Gestionar portales dinámicos
    gestionarPortalesEntorno(dt);

    // 2. Spawn de variantes de combate
    tiempoSpawn += dt;
    if (tiempoSpawn >= config.frecuenciaSpawn) {
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    // 3. Spawn de voladoras (proyectiles inteligentes)
    tiempoSpawnVoladoras += dt;
    if (tiempoSpawnVoladoras >= frecuenciaSpawnVoladoras) {
        spawnVoladora();
        tiempoSpawnVoladoras = 0.0f;
    }

    // 4. Coordinar táctica de enjambre
    QList<Variante*> enjambre;
    for (Enemigo* e : std::as_const(enemigos)) {
        Variante* v = dynamic_cast<Variante*>(e);
        if (v && !dynamic_cast<VariantePortal*>(v) &&
            v->isVisible() && v->getVida() > 0.0f) {
            enjambre.append(v);
        }
    }

    int cantidad = enjambre.size();
    if (cantidad > 0) {
        float separacionAngular = (2.0f * 3.14159f) / cantidad;
        bool hayAtacante = false;

        for (int i = 0; i < cantidad; ++i) {
            if (enjambre[i]->getEstado() == Variante::Estado::ATACAR)
                hayAtacante = true;
            enjambre[i]->setAngulo(i * separacionAngular);
        }

        if (!hayAtacante) {
            enjambre[rand() % cantidad]->setEstado(Variante::Estado::ATACAR);
        }
    }

    // 5. Actualizar todos los enemigos visibles
    for (Enemigo* e : std::as_const(enemigos)) {
        if (e->isVisible() && jugador && jugador->isActivo()) {
            e->update(dt, *jugador);
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

    for (Enemigo* e : std::as_const(enemigos)) {
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

            float dirX = (e->getX() > xJ) ? 1.0f : -1.0f;
            float dirY = (e->getY() > yJ) ? 1.0f : -1.0f;

            jugador->setPosicion(jugador->getX() - dirX * 15.0f,
                                 jugador->getY());

            AngstromLevy*   levy      = dynamic_cast<AngstromLevy*>(e);
            VariantePortal* varPortal = dynamic_cast<VariantePortal*>(e);

            if (levy) {
                levy->recibirImpacto(dirX);
            } else if (varPortal) {
                varPortal->setVelocidad(-varPortal->getVelX() * 1.5f,
                                        -varPortal->getVelY() * 1.5f);
                varPortal->setPosicion(varPortal->getX() + dirX * 30.0f,
                                       varPortal->getY() + dirY * 30.0f);
            } else {
                e->setPosicion(e->getX() + dirX * 50.0f,
                               e->getY() + dirY * 20.0f);
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

    for (Enemigo* e : std::as_const(enemigos)) {
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
            bool eraInvulnerable = jugador->isInvulnerable();
            jugador->recibirDanio(e->getDanio());

            if (!eraInvulnerable) {
                float dirX = (xJ > e->getX()) ? 1.0f : -1.0f;
                float dirY = (yJ > e->getY()) ? 1.0f : -1.0f;

                jugador->setPosicion(jugador->getX() + dirX * 45.0f,
                                     jugador->getY() + dirY * 15.0f);

                VariantePortal* varPortal = dynamic_cast<VariantePortal*>(e);
                if (varPortal) {
                    varPortal->setVelocidad(varPortal->getVelX() * -0.6f,
                                            varPortal->getVelY() *  0.8f);
                } else {
                    e->setPosicion(e->getX() - dirX * 12.0f, e->getY());
                }
            }

            // Aprendizaje del agente
            VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
            if (vp && !vp->getImpacto()) {
                vp->setImpacto(true);
                VariantePortal::aprender(vp->getZonaOrigen(), 1.0f);
            }
        }
    }

    // Daño ambiental del portal — solo Nivel 1
    if (nivelActivo == 1) {
        for (Portal* p : std::as_const(portales)) {
            if (!p->isActivo()) continue;
            if (p->jugadorTocaBorde(xJ, yJ, 60.0f, 80.0f)) {
                jugador->recibirDanioAmbiental(config.danioPortal * 0.016f);
            }
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
    // VariantePortal que salió de pantalla — aprende del resultado
    for (Enemigo* e : std::as_const(enemigos)) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
        if (vp && vp->getVida() > 0.0f &&
            vp->getVelX() == 0.0f && !vp->isVisible()) {
            if (vp->getZonaOrigen() != -1) {
                if (!vp->getImpacto()) {
                    VariantePortal::aprender(vp->getZonaOrigen(), 0.0f);
                }
                vp->setImpacto(true);
                vp->setZonaOrigen(-1);
            }
        }
    }

    for (int i = enemigos.size() - 1; i >= 0; i--) {
        if (!enemigos[i]->isActivo()) {
            VariantePortal* vp = dynamic_cast<VariantePortal*>(enemigos[i]);
            if (vp && !vp->getImpacto()) {
                vp->setImpacto(true);
                VariantePortal::aprender(vp->getZonaOrigen(), 0.0f);
            }
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

    if (nivelActivo == 2) {
        // Victoria: todas las variantes derrotadas
        bool hayEnemigosVivos = false;
        for (Enemigo* e : std::as_const(enemigos)) {
            if (e->getVida() > 0.0f) {
                hayEnemigosVivos = true;
                break;
            }
        }
        if (totalVariantesSpawneadas >= 6 && !hayEnemigosVivos) {
            estado.setEstado(GameState::EstadoPartida::VICTORIA);
            return;
        }

        // Derrota: se acabó el tiempo
        if (tiempoNivel2 <= 0.0f) {
            estado.setEstado(GameState::EstadoPartida::DERROTA);
            return;
        }
    }
}

// ── spawnVariante ────────────────────────────────────────────────
void GameEngine::spawnVariante()
{
    for (Enemigo* e : std::as_const(enemigos)) {
        Variante* v = dynamic_cast<Variante*>(e);
        if (v && !dynamic_cast<VariantePortal*>(v) &&
            !v->isVisible() && v->getVida() > 0.0f) {
            if (!portales.isEmpty()) {
                int idx = rand() % portales.size();
                v->setPosicion(portales[idx]->getX(),
                               portales[idx]->getY());
            } else {
                v->setPosicion(100.0f + (rand() % 600),
                               100.0f + (rand() % 400));
            }
            v->show();
            totalVariantesSpawneadas++;
            return;
        }
    }
}

// ── spawnVoladora ────────────────────────────────────────────────
void GameEngine::spawnVoladora()
{
    if (portales.isEmpty()) return;

    for (Enemigo* e : std::as_const(enemigos)) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
        if (vp && !vp->isVisible() &&
            vp->getVida() > 0.0f && vp->getZonaOrigen() == -1) {

            int zonaElegida       = VariantePortal::razonar();
            QPointF posObjetivo   = VariantePortal::obtenerCoordenadaPortal(zonaElegida);
            int idx               = rand() % portales.size();
            Portal* portalCuna    = portales[idx];

            vp->setPosicion(portalCuna->getX(), portalCuna->getY());
            vp->setZonaOrigen(zonaElegida);
            vp->setImpacto(false);

            float dx       = posObjetivo.x() - vp->getX();
            float dy       = posObjetivo.y() - vp->getY();
            float magnitud = std::sqrt(dx * dx + dy * dy);

            if (magnitud > 0.0f) {
                vp->setVelocidad((dx / magnitud) * 370.0f,
                                 (dy / magnitud) * 370.0f);
            }
            vp->show();
            totalVariantesSpawneadas++;
            return;
        }
    }
}

// ── gestionarPortalesEntorno ─────────────────────────────────────
void GameEngine::gestionarPortalesEntorno(float dt)
{
    int voladorasVivas = 0;
    for (Enemigo* e : std::as_const(enemigos)) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
        if (vp && vp->getVida() > 0.0f) voladorasVivas++;
    }

    int minPortales = 0, maxPortales = 0;
    if (voladorasVivas >= 2) {
        minPortales = 5; maxPortales = 7;
    } else if (voladorasVivas == 1) {
        minPortales = 2; maxPortales = 4;
    } else {
        for (Portal* p : std::as_const(portales)) delete p;
        portales.clear();
        targetPortales = 0;
        return;
    }

    if (targetPortales < minPortales) targetPortales = minPortales;
    if (targetPortales > maxPortales) targetPortales = maxPortales;

    tiempoFluctuacionPortales += dt;
    if (tiempoFluctuacionPortales > 1.0f) {
        tiempoFluctuacionPortales = 0.0f;
        if (creciendoPortales) {
            targetPortales++;
            if (targetPortales >= maxPortales) creciendoPortales = false;
        } else {
            targetPortales--;
            if (targetPortales <= minPortales) creciendoPortales = true;
        }
    }

    // Crear portales nuevos — sin escena, MainWindow los agrega al sincronizar
    while (portales.size() < targetPortales) {
        float fase = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        Portal* p = new Portal(380.0f, 250.0f, 320.0f, 1.0f, fase);
        portales.append(p);
    }
    while (portales.size() > targetPortales && !portales.isEmpty()) {
        delete portales.takeFirst();
    }

    for (Portal* p : std::as_const(portales)) {
        p->updateConPhysics(dt, physics);
    }
}

// ── Getters ──────────────────────────────────────────────────────
void GameEngine::pausar()   { pausado = true;  }
void GameEngine::reanudar() { pausado = false; }

const GameState&       GameEngine::getEstado()   const { return estado;   }
Jugador*               GameEngine::getJugador()  const { return jugador;  }
const QList<Enemigo*>& GameEngine::getEnemigos() const { return enemigos; }
const QList<Portal*>&  GameEngine::getPortales() const { return portales; }
