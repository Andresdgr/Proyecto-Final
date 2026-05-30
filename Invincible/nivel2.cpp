#include "nivel2.h"
#include "jugador.h"
#include <QGraphicsPixmapItem>
#include <utility>

Nivel2::Nivel2(QGraphicsScene* escenaCompartida)
    : Nivel(escenaCompartida),
    tiempoRestante(60.0f),
    tiempoSpawn(0.0f),
    frecuenciaSpawn(4.0f)
{
    // El constructor de Nivel ya inicializó 'escena' y arrancó 'gameTimer' a 60 FPS.
}

Nivel2::~Nivel2() {
    qDeleteAll(clones);
    clones.clear();

    qDeleteAll(portales);
    portales.clear();

    // 'jugador' y 'gameTimer' se eliminan en el destructor de la clase base Nivel.
}

void Nivel2::inicializarEscenario() {
    // 1. Cargar fondo del nivel
    QGraphicsPixmapItem* fondo = new QGraphicsPixmapItem(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Nivel_2.png"));
    escena->addItem(fondo);

    // 2. Instanciar jugador (el puntero 'jugador' está protegido en la clase Nivel)
    jugador = new Jugador(400.0f, 300.0f);
    escena->addItem(jugador);
}

void Nivel2::actualizarCicloJuego() {
    // El QTimer en nivel.cpp corre a 16ms, por lo tanto dt es aprox 0.016 segundos.
    float dt = 0.016f;

    // 1. Lógica de supervivencia
    tiempoRestante -= dt;
    if (tiempoRestante < 0.0f) tiempoRestante = 0.0f;

    tiempoSpawn += dt;
    if (tiempoSpawn >= frecuenciaSpawn) {
        spawnClon();
        tiempoSpawn = 0.0f;
    }

    // 2. Actualizar Jugador
    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
    }

    // 3. Actualizar Enemigos
    for (Enemigo* c : std::as_const(clones)) {
        if (c->isActivo() && jugador) {
            c->update(dt, *jugador);
        }
    }

    // 4. Actualizar Portales
    for (Portal* p : std::as_const(portales)) {
        if (p->isActivo()) {
            // Requiere inyectar PhysicsEngine si se maneja aquí
            // p->updateConPhysics(dt, physics);
        }
    }

    verificarColisiones();
    limpiarInactivos();
}

void Nivel2::verificarColisiones() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    for (Enemigo* c : std::as_const(clones)) {
        if (!c->isActivo()) continue;

        // Lógica AABB adaptada de tu GameEngine
        if (std::abs(xJ - c->getX()) < 60.0f && std::abs(yJ - c->getY()) < 60.0f) {
            jugador->recibirDanio(c->getDanio());
            if (jugador->getDanioActual() > 0.0f) {
                c->recibirDanio(jugador->getDanioActual());
            }
        }
    }
}

void Nivel2::spawnClon() {
    // Implementar la instanciación de un nuevo enemigo en coordenadas aleatorias
}

void Nivel2::limpiarInactivos() {
    for (int i = clones.size() - 1; i >= 0; i--) {
        if (!clones[i]->isActivo()) {
            delete clones[i];
            clones.removeAt(i);
        }
    }
}
