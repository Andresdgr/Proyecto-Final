#include "gameengine.h"
#include "nivel2.h"
// #include "nivel1.h"
#include <stdexcept>
// ── Constructor ─────────────────────────────────────────────────
GameEngine::GameEngine(const DifficultyConfig& config)
    : nivelActual(nullptr)
    , physics(config)
    , config(config)
    , idNivelActivo(0)
    , pausado(false)
{
}

// ── Destructor — libera toda la memoria dinámica ─────────────────
GameEngine::~GameEngine()
{
    // Al eliminar nivelActual, el destructor de la clase hija (Nivel2)
    // se encargará de limpiar sus propios enemigos, portales y el jugador.
    delete nivelActual;
    nivelActual = nullptr;
}

// ── iniciarNivel ────────────────────────────────────────────────
void GameEngine::iniciarNivel(uint8_t nivel, QGraphicsScene* escenaCompartida)
{
    // 1. Limpiar el escenario del nivel previo si existía alguno activo
    if (nivelActual) {
        delete nivelActual;
        nivelActual = nullptr;
    }

    idNivelActivo = nivel;
    estado.setNivel(nivel);
    estado.setEstado(GameState::EstadoPartida::JUGANDO);

    // 2. NUMERAL 2: Instanciación polimórfica según el nivel seleccionado
    if (nivel == 1) {
        // nivelActual = new Nivel1(escenaCompartida);
    }
    else if (nivel == 2) {
        nivelActual = new Nivel2(escenaCompartida);
    }
    else {
        throw std::invalid_argument(
            "GameEngine::iniciarNivel — nivel debe ser 1 o 2");
    }

    // 3. Ejecutar la carga de elementos del escenario concreto
    if (nivelActual) {
        nivelActual->inicializarEscenario();
    }
}

// ── update — NUMERAL 3: Delegación del ciclo principal ──────────
void GameEngine::update(float dt)
{
    if (pausado) return;
    if (estado.getEstado() != GameState::EstadoPartida::JUGANDO) return;

    physics.setDt(dt);



    actualizarGameState();
    verificarCondicionFin();
}

// ── Actualizar GameState ─────────────────────────────────────────
void GameEngine::actualizarGameState()
{
    Jugador* jugador = getJugador();
    if (jugador) {
        estado.setVidaJugador(jugador->getVida(),
                              jugador->getVidaMaxima());
        estado.setHitStreak(jugador->getHitStreak());
        estado.setComboActivo(jugador->isComboActivo());
    }

}

// ── Verificar condición de fin ───────────────────────────────────
void GameEngine::verificarCondicionFin()
{
    Jugador* jugador = getJugador();
    if (!jugador || !jugador->isActivo()) {
        estado.setEstado(GameState::EstadoPartida::DERROTA);
        return;
    }

    // Las condiciones específicas de victoria se gestionan internamente en las clases hijas.
    // Si el nivel2 determina que el tiempo acabó, modificará el estado correspondiente.
}

// ── Pausar y reanudar ────────────────────────────────────────────
void GameEngine::pausar()   { pausado = true;  }
void GameEngine::reanudar() { pausado = false; }

// ── Getter Polimórfico ───────────────────────────────────────────
Jugador* GameEngine::getJugador() const
{
    return nivelActual ? nivelActual->obtenerJugador() : nullptr;
}
