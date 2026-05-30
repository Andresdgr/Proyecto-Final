#ifndef NIVEL_H
#define NIVEL_H

#include <QGraphicsScene>
#include <QTimer>
#include <QObject>
#include <vector>
#include "jugador.h"

class Nivel : public QObject {
    Q_OBJECT
protected:
    QGraphicsScene* escena;
    QTimer* gameTimer;
    Jugador* jugador;
    // std::vector<Enemigo*> listaEnemigos; // Memoria dinámica compartida, hace falta crear el objeto enemigo

public:
    Nivel(QGraphicsScene* escenaCompartida);
    virtual ~Nivel();

    QGraphicsScene* getEscena() const { return escena; }
    Jugador* obtenerJugador() const { return jugador; }
    // Métodos obligatorios para cada nivel
    virtual void inicializarEscenario() = 0;
    virtual void verificarColisiones() = 0;

public slots:
    virtual void actualizarCicloJuego() = 0; // Se ejecuta en cada tick del QTimer
};

#endif // NIVEL_H
