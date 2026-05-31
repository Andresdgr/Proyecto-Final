#ifndef NIVEL2_H
#define NIVEL2_H

#include "nivel.h"
#include "enemigo.h"
#include "portal.h"
#include "physicsengine.h"
#include <QList>

class Nivel2 : public Nivel {
    Q_OBJECT

public:
    explicit Nivel2(QGraphicsScene* escenaCompartida);
    ~Nivel2() override;

    // Métodos obligatorios heredados de Nivel
    void inicializarEscenario() override;
    void verificarColisiones() override;
    void verificarAtaqueJugador();

public slots:
    // Slot obligatorio ejecutado por el QTimer de Nivel
    void actualizarCicloJuego() override;

private:
    PhysicsEngine* physics;
    // Contenedores específicos del Nivel 2 extraídos del GameEngine
    QList<Enemigo*> variantes;
    QList<Portal*> portales;

    // Variables de control de tiempo del Nivel 2
    float tiempoRestante;
    float tiempoSpawn;
    float frecuenciaSpawn;

    // Métodos internos
    void spawnVariante();
    void limpiarInactivos();
};

#endif // NIVEL2_H
