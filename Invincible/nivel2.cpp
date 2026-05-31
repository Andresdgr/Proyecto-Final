#include "nivel2.h"
#include "jugador.h"
#include "variante.h"
#include <QGraphicsPixmapItem>
#include <utility>
#include "physicsengine.h"
#include "difficultyconfig.h"
#include <cmath> // Para std::abs
#include <cstdlib> // Para rand()

Nivel2::Nivel2(QGraphicsScene* escenaCompartida)
    : Nivel(escenaCompartida),
    tiempoRestante(60.0f),
    tiempoSpawn(0.0f),
    frecuenciaSpawn(4.0f)
{
    physics = new PhysicsEngine(DifficultyConfig::normal());
}

Nivel2::~Nivel2() {
    qDeleteAll(variantes);
    variantes.clear();

    qDeleteAll(portales);
    portales.clear();

    // 'jugador' y 'gameTimer' se eliminan en el destructor de la clase base Nivel.
    delete physics;
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
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    // 2. Actualizar Jugador
    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
    }

    // 3. Actualizar Enemigos
    for (Enemigo* c : std::as_const(variantes)) {
        if (c->isActivo() && jugador) {
            c->update(dt, *jugador);

            // Si el clon fue golpeado, recupera su opacidad gradualmente
            if (c->opacity() < 1.0f) {
                c->setOpacity(c->opacity() + dt * 4.0f); // Retorna al estado sólido (1.0)
                if (c->opacity() > 1.0f) c->setOpacity(1.0f);
            }
        }
    }

    // 4. Actualizar Portales
    for (Portal* p : std::as_const(portales)) {
        if (p->isActivo()) {
            // Requiere inyectar PhysicsEngine si se maneja aquí
            p->updateConPhysics(dt, *physics);
        }
    }

    verificarColisiones();
    limpiarInactivos();
}

void Nivel2::verificarColisiones() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    for (Enemigo* c : std::as_const(variantes)) {
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

void Nivel2::spawnVariante() {
    // 1. Definir coordenadas aleatorias dentro de los límites de la escena
    float xAleatorio = 100.0f + (rand() % 600);
    float yAleatorio = 100.0f + (rand() % 400);

    // 2. Instanciar el enemigo
    Enemigo* nuevaVariante = new Variante(xAleatorio, yAleatorio, 100.0f, 75.0f, 10.0f, 100);

    // 3. Vincular el Sprite de la otra dimensión
    nuevaVariante->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Viltrumincible_85x85.png"));
    nuevaVariante->setPos(xAleatorio, yAleatorio);

    // 4. Registrar en el contenedor lógico y añadir a la escena de Qt
    variantes.append(nuevaVariante);
    escena->addItem(nuevaVariante);
}

void Nivel2::verificarAtaqueJugador() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();
    float danio = jugador->getDanioActual();

    for (Enemigo* c : std::as_const(variantes)) {
        if (!c->isActivo()) continue;

        // Determinar proximidad en un rango de ataque (ej. 80 píxeles)
        if (std::abs(xJ - c->getX()) < 80.0f && std::abs(yJ - c->getY()) < 80.0f) {
            c->recibirDanio(danio);

            // VISUALIZACIÓN DEL DAÑO:
            c->setOpacity(0.3f);
        }
    }
}

void Nivel2::limpiarInactivos() {
    for (int i = variantes.size() - 1; i >= 0; i--) {
        if (!variantes[i]->isActivo()) {
            delete variantes[i];
            variantes.removeAt(i);
        }
    }
}
