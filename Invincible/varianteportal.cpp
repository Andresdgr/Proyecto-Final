#include "varianteportal.h"
#include <algorithm>

float VariantePortal::pesosZonas[9] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
int VariantePortal::zonaActualJugador = 4;

VariantePortal::VariantePortal(float x, float y, float vida, float masa, float danio, uint16_t puntos)
    : Enemigo(x, y, vida, masa, danio, puntos) {}

void VariantePortal::update(float dt) {
    moverse(dt);
}

void VariantePortal::update(float dt, const Jugador& jugador) {
    // Ignoramos la posición del jugador en tiempo real.
    // Su vector de velocidad (velX, velY) se le asigna solo una vez al momento de nacer.
    moverse(dt);
}

void VariantePortal::moverse(float dt) {

    // Si su velocidad es cero, está escondida esperando que el Agente abra un portal
    if (velX == 0.0f && velY == 0.0f) return;

    x += velX * dt;
    y += velY * dt;

    // Si sale de los límites de la pantalla
    if (x < -85.0f || x > 800.0f || y < -85.0f || y > 600.0f) {
        velX = 0.0f;
        velY = 0.0f;
        hide(); // Se oculta visualmente, pero sigue viva para el próximo portal
    }

    setPosicion(x, y);
}

// Implementación del Agente Inteligente

void VariantePortal::percibir(float xJugador, float yJugador) {
    int col = static_cast<int>(xJugador / ANCHO_TERCIO);
    int fila = static_cast<int>(yJugador / ALTO_TERCIO);

    col = std::clamp(col, 0, 2);
    fila = std::clamp(fila, 0, 2);
    zonaActualJugador = col + (fila * 3);
}

int VariantePortal::razonar() {
    int zonaOptima = 0;
    float maxPeso = -1.0f;

    for (int i = 0; i < 9; ++i) {
        if (pesosZonas[i] > maxPeso) {
            maxPeso = pesosZonas[i];
            zonaOptima = i;
        }
    }
    return zonaOptima;
}

void VariantePortal::aprender(int zona, float recompensa) {
    pesosZonas[zona] = pesosZonas[zona] * (1.0f - alpha) + recompensa * alpha;
}

QPointF VariantePortal::obtenerCoordenadaPortal(int zona) {
    int col = zona % 3;
    int fila = zona / 3;

    float xCentro = (col * ANCHO_TERCIO) + (ANCHO_TERCIO / 2.0f);
    float yCentro = (fila * ALTO_TERCIO) + (ALTO_TERCIO / 2.0f);

    return QPointF(xCentro, yCentro);
}
