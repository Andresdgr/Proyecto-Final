#include "nivel2.h"
#include "jugador.h"
#include "variante.h"
#include "varianteportal.h"
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
    frecuenciaSpawn(4.0f),
    indiceSecuencia(0)
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
    if (tiempoSpawn >= frecuenciaSpawn && variantes.size() < MAX_SIMULTANEOS) {
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
    if (variantes.size() >= MAX_SIMULTANEOS) return;

    Enemigo* nuevoEnemigo = nullptr;

    // Posiciones 3 y 6 en la secuencia (índices 2 y 5) corresponden a la Variante de Portal
    if (indiceSecuencia == 2 || indiceSecuencia == 5) {

        // 1. Crear el portal orbital en el centro
        float faseAleatoria = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        Portal* nuevoPortal = new Portal(400.0f, 300.0f, 290.0f, 1.0f, faseAleatoria);
        portales.append(nuevoPortal);
        escena->addItem(nuevoPortal);

        // 2. Instanciar la nueva VariantePortal
        VariantePortal* varianteVoladora = new VariantePortal(nuevoPortal->getX(), nuevoPortal->getY(), 100.0f, 75.0f, 15.0f, 150);
        varianteVoladora->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Capevincible_85x85.png"));

        // 3. Asignar el vector de velocidad inicial hacia el jugador
        if (jugador && jugador->isActivo()) {
            float dx = jugador->getX() - varianteVoladora->getX();
            float dy = jugador->getY() - varianteVoladora->getY();
            float magnitud = std::sqrt(dx*dx + dy*dy);

            if (magnitud > 0.0f) {
                // Velocidad alta (ej. 250 px/s) para que atraviese el mapa rápido
                varianteVoladora->setVelocidad((dx / magnitud) * 250.0f, (dy / magnitud) * 250.0f);
            }
        }
        nuevoEnemigo = varianteVoladora;

    } else {
        // Variantes normales cuerpo a cuerpo
        float xAleatorio = 100.0f + (rand() % 600);
        float yAleatorio = 100.0f + (rand() % 400);

        nuevoEnemigo = new Variante(xAleatorio, yAleatorio, 100.0f, 75.0f, 10.0f, 100);
        nuevoEnemigo->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Viltrumincible_85x85.png"));
    }

    // Registrar y añadir a la escena
    nuevoEnemigo->setPos(nuevoEnemigo->getX(), nuevoEnemigo->getY());
    variantes.append(nuevoEnemigo);
    escena->addItem(nuevoEnemigo);

    // Repetir el patrón (0, 1, 2, 3, 4, 5, 0, 1...)
    indiceSecuencia = (indiceSecuencia + 1) % 6;
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
