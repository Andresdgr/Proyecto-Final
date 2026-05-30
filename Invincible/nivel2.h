#ifndef NIVEL2_H
#define NIVEL2_H

#include "nivel.h"
#include "enemigo.h"
#include "portal.h"
#include <QList> // Puedes reemplazar QList por tu plantilla de estructura de datos personalizada (Lista/Nodo)

class Nivel2 : public Nivel {
    Q_OBJECT

public:
    explicit Nivel2(QGraphicsScene* escenaCompartida);
    ~Nivel2() override;

    // Métodos obligatorios heredados de Nivel
    void inicializarEscenario() override;
    void verificarColisiones() override;

public slots:
    // Slot obligatorio ejecutado por el QTimer de Nivel
    void actualizarCicloJuego() override;

private:
    // Contenedores específicos del Nivel 2 extraídos del GameEngine
    QList<Enemigo*> clones;
    QList<Portal*> portales;

    // Variables de control de tiempo del Nivel 2
    float tiempoRestante;
    float tiempoSpawn;
    float frecuenciaSpawn;

    // Métodos internos
    void spawnClon();
    void limpiarInactivos();
};

#endif // NIVEL2_H
