#ifndef NIVEL2_H
#define NIVEL2_H

#include "nivel.h"
#include "enemigo.h"
#include "portal.h"
#include "physicsengine.h"
#include "agenteinteligente.h"
#include <QList>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

class Nivel2 : public Nivel {
    Q_OBJECT

public:
    explicit Nivel2(QGraphicsScene* escenaCompartida);
    ~Nivel2() override;

    // Métodos obligatorios heredados de Nivel
    void inicializarEscenario() override;
    void verificarColisiones() override;
    void verificarAtaqueJugador() override;
    float getTiempoRestante() const override { return tiempoRestante; }
    bool nivelCompletado() const override;

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
    float tiempoSpawnVoladoras;  // Timer exclusivo para voladoras
    float frecuenciaSpawnVoladoras;

    // Variables para la fluctuación de portales
    float tiempoFluctuacionPortales = 0.0f;
    int targetPortales = 0;
    bool creciendoPortales = true;

    // Control de la secuencia de invasión
    uint8_t indiceSecuencia;
    static constexpr uint8_t MAX_SIMULTANEOS = 6;

    // Métodos internos
    void spawnVariante();
    void spawnVoladora();
    void limpiarInactivos();
    void verificarVictoria();
    void gestionarPortalesEntorno(float dt);

    AgenteInteligente* agente;

    // --- UI (Interfaz Gráfica) ---
    QGraphicsRectItem* barraFondoJugador;
    QGraphicsRectItem* barraVidaJugador;
    QGraphicsTextItem* textoTiempo;

    // Arreglos de tamaño 6 para manejar el Grid de las variantes
    QGraphicsRectItem* barrasFondoVariantes[6];
    QGraphicsRectItem* barrasVidaVariantes[6];

    void inicializarUI();
    void actualizarUI();

    // El escuadrón fijo de 6 variantes
    Enemigo* misSeisVariantes[6];
};

#endif // NIVEL2_H
