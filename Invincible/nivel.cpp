#include "nivel.h"

Nivel::Nivel(QGraphicsScene* escenaCompartida) {
    this->escena = escenaCompartida;
    this->jugador = nullptr; // Se instanciará en las clases hijas (Nivel1 o Nivel2)

    // Configuración del bucle central del juego a 60 FPS (~16 ms por frame)
    gameTimer = new QTimer();
    connect(gameTimer, &QTimer::timeout, this, &Nivel::actualizarCicloJuego);
    gameTimer->start(16);
}

Nivel::~Nivel() {
    // Parar el timer
    gameTimer->stop();
    delete gameTimer;
    // Liberar memoria dinámica del jugador y de los contenedores de enemigos (clones, portales, etc.)

}
