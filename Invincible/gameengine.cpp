#include "gameengine.h"
#include "angstromlevy.h"
#include "variante.h"
#include "varianteportal.h"
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <QGraphicsScene>
#include <utility>

// Constructor
GameEngine::GameEngine(const DifficultyConfig& config)
    : jugador(nullptr)
    , physics(config)
    , config(config)
    , tiempoNivel2(60.0f)
    , tiempoSpawn(0.0f)
    , tiempoVariantePortal(0.0f)
    , nivelActivo(0)
    , pausado(false)
    , escena(nullptr)
    , totalVariantesSpawneadas(0)
{
    for (int i = 0; i < 6; ++i) misSeisVariantes[i] = nullptr;
}

// Destructor
GameEngine::~GameEngine()
{
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : std::as_const(enemigos)) delete e;
    enemigos.clear();

    for (Portal* p : std::as_const(portales)) delete p;
    portales.clear();
}

// iniciarNivel
void GameEngine::iniciarNivel(uint8_t nivel, QGraphicsScene* escena)
{
    this->escena = escena;
    delete jugador;
    jugador = nullptr;

    for (Enemigo* e : std::as_const(enemigos)) delete e;
    enemigos.clear();

    for (Portal* p : std::as_const(portales)) delete p;
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
        tiempoNivel2 = 90.0f; // Tiempo de supervivencia
        tiempoSpawn  = 0.0f;
        tiempoSpawnVoladoras = 0.0f;
        frecuenciaSpawnVoladoras = 1.0f; // Disparan cada 1s
        tiempoFluctuacionPortales = 0.0f;
        targetPortales = 0;
        creciendoPortales = true;
        estado.setTiempoRestante(tiempoNivel2);

        // Instanciar exactamente las 6 variantes
        for (int i = 0; i < 6; ++i) {
            Enemigo* v = nullptr;
            if (i == 2 || i == 5) {
                // Las 2 variantes voladoras (agentes)
                v = new VariantePortal(-1000.0f, -1000.0f, 300.0f, 75.0f, 15.0f, 150);
            } else {
                // Las 4 variantes de combate normal
                v = new Variante(-1000.0f, -1000.0f, 500.0f, 75.0f, 10.0f, 100);
            }
            v->hide(); // Ocultas por defecto
            enemigos.append(v);
            misSeisVariantes[i] = v;
        }
    } else {
        throw std::invalid_argument(
            "GameEngine::iniciarNivel — nivel debe ser 1 o 2");
    }
}

// update
void GameEngine::update(float dt)
{
    if (pausado) return;
    if (estado.getEstado() != GameState::EstadoPartida::JUGANDO) return;

    physics.setDt(dt);

    if (jugador && jugador->isActivo()) {
        jugador->update(dt);

        VariantePortal::percibir(jugador->getX(), jugador->getY());
    }

    if      (nivelActivo == 1) actualizarNivel1(dt);
    else if (nivelActivo == 2) actualizarNivel2(dt);

    verificarColisiones();
    limpiarInactivos();
    actualizarGameState();
    verificarCondicionFin();
}

//  Nivel 1
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

// Nivel 2
void GameEngine::actualizarNivel2(float dt)
{
    tiempoNivel2 -= dt;
    if (tiempoNivel2 < 0.0f) tiempoNivel2 = 0.0f;
    estado.setTiempoRestante(tiempoNivel2);

    // 1. Dinámica de los portales orbitales
    gestionarPortalesEntorno(dt);

    // 2. Temporizadores de Spawn separados
    tiempoSpawn += dt;
    if (tiempoSpawn >= config.frecuenciaSpawn) {
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    tiempoSpawnVoladoras += dt;
    if (tiempoSpawnVoladoras >= frecuenciaSpawnVoladoras) {
        spawnVoladora();
        tiempoSpawnVoladoras = 0.0f;
    }

    // 3. Coordinar Táctica de Enjambre
    QList<Variante*> enjambre;

    // Censar variantes vivas en pantalla
    for (Enemigo* e : std::as_const(enemigos)) {
        Variante* v = dynamic_cast<Variante*>(e);
        if (v && !dynamic_cast<VariantePortal*>(v) && v->isVisible() && v->getVida() > 0.0f) {
            enjambre.append(v);
        }
    }

    int cantidad = enjambre.size();
    if (cantidad > 0) {
        // Reparte el círculo alrededor del jugador
        float separacionAngular = (2.0f * 3.14159f) / cantidad;
        bool hayAtacante = false;

        for (int i = 0; i < cantidad; ++i) {
            if (enjambre[i]->getEstado() == Variante::Estado::ATACAR) hayAtacante = true;
            enjambre[i]->setAngulo(i * separacionAngular); // Asigna posición en el cerco [cite: 341]
        }

        // Otorgar el turno de ataque si nadie lo tiene
        if (!hayAtacante) {
            enjambre[rand() % cantidad]->setEstado(Variante::Estado::ATACAR);
        }
    }

    // 4. Actualizar estado de los enemigos en pantalla
    for (Enemigo* e : std::as_const(enemigos)) {
        if (e->isVisible() && jugador && jugador->isActivo()) {
            e->update(dt, *jugador);
        }
    }
}

// aplicarAtaqueJugador
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

            // Impulso parabólico solo para AngstromLevy
            AngstromLevy* levy = dynamic_cast<AngstromLevy*>(e);
            if (levy) {
                float dirX = (e->getX() > xJ) ? 1.0f : -1.0f;
                levy->recibirImpacto(dirX);
            }
        }
    }
}

// verificarColisiones
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
            jugador->recibirDanio(e->getDanio());

            // Aprendizaje
            VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
            if (vp && !vp->getImpacto()) {
                vp->setImpacto(true); // Evita sumar doble recompensa
                VariantePortal::aprender(vp->getZonaOrigen(), 1.0f);
            }
        }
    }

    // Portal — daño ambiental continuo
    if (nivelActivo == 1) {
        for (Portal* p : std::as_const(portales)) {
            if (!p->isActivo()) continue;

            if (p->jugadorTocaBorde(xJ, yJ, 60.0f, 80.0f)) {
                jugador->recibirDanioAmbiental(config.danioPortal * 0.016f);
            }
        }
    }
}

// colisionAABB
bool GameEngine::colisionAABB(float x1, float y1,
                              float x2, float y2,
                              float semi1, float semi2) const
{
    return (std::abs(x1 - x2) < (semi1 + semi2)) &&
           (std::abs(y1 - y2) < (semi1 + semi2));
}

//  limpiarInactivos
void GameEngine::limpiarInactivos()
{
    // VariantePortal viva pero que salió de pantalla (vel==0 y oculta)
    for (Enemigo* e : std::as_const(enemigos)) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
        if (vp && vp->getVida() > 0.0f && vp->getVelX() == 0.0f && !vp->isVisible()) {
            if (vp->getZonaOrigen() != -1) {
                if (!vp->getImpacto()) {
                    VariantePortal::aprender(vp->getZonaOrigen(), 0.0f);
                }
                vp->setImpacto(true);
                vp->setZonaOrigen(-1);   // Permite que sea relanzada
            }
        }
    }

    for (int i = enemigos.size() - 1; i >= 0; i--) {
        if (!enemigos[i]->isActivo()) {

            // El agente aprende del error
            VariantePortal* vp = dynamic_cast<VariantePortal*>(enemigos[i]);
            if (vp && !vp->getImpacto()) {
                vp->setImpacto(true); // Evita doble castigo por error
                VariantePortal::aprender(vp->getZonaOrigen(), 0.0f);
            }

            estado.sumarPuntos(enemigos[i]->getPuntosAlDerrotar());
            if (nivelActivo == 2) {
                for (int j = 0; j < 6; ++j) {
                    if (misSeisVariantes[j] == enemigos[i]) {
                        misSeisVariantes[j] = nullptr;
                        break;
                    }
                }
            }
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

// actualizarGameState
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

//  verificarCondicionFin
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
        if (totalVariantesSpawneadas >= 6) {
            bool todasDerrotadas = true;
            for (int i = 0; i < 6; ++i) {
                if (misSeisVariantes[i] != nullptr && misSeisVariantes[i]->getVida() > 0.0f) {
                    todasDerrotadas = false;
                    break;
                }
            }
            if (todasDerrotadas) {
                estado.setEstado(GameState::EstadoPartida::VICTORIA);
                return;
            }
        }


        // CONDICIÓN DE DERROTA: Se acabó el tiempo y aún quedan enemigos en el mapa
        if (tiempoNivel2 <= 0.0f) {
            estado.setEstado(GameState::EstadoPartida::DERROTA);
            return;
        }
    }
}

//  spawnVariante
void GameEngine::spawnVariante() {
    for (Enemigo* e : std::as_const(enemigos)) {
        Variante* v = dynamic_cast<Variante*>(e);
        if (v && !dynamic_cast<VariantePortal*>(v) && !v->isVisible() && v->getVida() > 0.0f) {
            if (!portales.isEmpty()) {
                int indicePortal = rand() % portales.size();
                Portal* portalCuna = portales[indicePortal];

                // Nace exactamente donde está flotando el portal
                v->setPosicion(portalCuna->getX(), portalCuna->getY());
            } else {
                // Respaldo de seguridad si no hay portales
                float xAleatorio = 100.0f + (rand() % 600);
                float yAleatorio = 100.0f + (rand() % 400);
                v->setPosicion(xAleatorio, yAleatorio);
            }

            v->show();
            totalVariantesSpawneadas++;
            return;
        }
    }
}

//  pausar / reanudar
void GameEngine::pausar()   { pausado = true;  }
void GameEngine::reanudar() { pausado = false; }

// Getters
const GameState&       GameEngine::getEstado()   const { return estado;   }
Jugador*               GameEngine::getJugador()  const { return jugador;  }
const QList<Enemigo*>& GameEngine::getEnemigos() const { return enemigos; }
const QList<Portal*>&  GameEngine::getPortales() const { return portales; }


void GameEngine::spawnVoladora() {
    if (portales.isEmpty()) return;

    // Busca una VariantePortal oculta y viva
    for (Enemigo* e : std::as_const(enemigos)) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(e);

        // Condiciones: viva, oculta, y NUNCA lanzada antes en este ciclo (zonaOrigen == -1)
        if (vp && !vp->isVisible() && vp->getVida() > 0.0f && vp->getZonaOrigen() == -1) {

            // Usamos la mente colmena estática
            int zonaElegida = VariantePortal::razonar();
            QPointF posObjetivo = VariantePortal::obtenerCoordenadaPortal(zonaElegida);

            int indicePortal = rand() % portales.size();
            Portal* portalCuna = portales[indicePortal];

            vp->setPosicion(portalCuna->getX(), portalCuna->getY());
            vp->setZonaOrigen(zonaElegida); // Marca que fue lanzada
            vp->setImpacto(false);

            float dx = posObjetivo.x() - vp->getX();
            float dy = posObjetivo.y() - vp->getY();
            float magnitud = std::sqrt(dx*dx + dy*dy);

            if (magnitud > 0.0f) {
                vp->setVelocidad((dx / magnitud) * 370.0f, (dy / magnitud) * 370.0f);
            }
            vp->show();
            totalVariantesSpawneadas++;
            return;
        }
    }
}

void GameEngine::gestionarPortalesEntorno(float dt) {
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
        // Colapsar todo si mataste a los francotiradores
        for (Portal* p : std::as_const(portales)) delete p;
        portales.clear();
        targetPortales = 0;
        return;
    }

    if (targetPortales < minPortales) targetPortales = minPortales;
    if (targetPortales > maxPortales) targetPortales = maxPortales;

    // Fluctuación rítmica cada segundo
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

    // Instanciar o borrar portales
    while (portales.size() < targetPortales) {
        float faseAleatoria = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        Portal* p = new Portal(380.0f, 250.0f, 320.0f, 1.0f, faseAleatoria);
        portales.append(p);
        if (escena) escena->addItem(p);
    }
    while (portales.size() > targetPortales && !portales.isEmpty()) {
        delete portales.takeFirst();
    }

    // Actualizar movimiento
    for (Portal* p : std::as_const(portales)) {
        p->updateConPhysics(dt, physics);
    }
}
