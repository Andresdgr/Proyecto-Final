#include "agenteinteligente.h"
#include <algorithm>

AgenteInteligente::AgenteInteligente() : alpha(0.3f), zonaActualJugador(4) {
    // Inicializar todos los pesos neutros (0.5) al inicio de la partida
    for(int i = 0; i < 9; ++i) {
        pesosZonas[i] = 0.5f;
    }
}

void AgenteInteligente::percibir(float xJugador, float yJugador) {
    // Calcular columnas y filas
    // zonaActual = (posJugador.x/ancho_tercio) + (posJugador.y/alto_tercio) * 3;
    int col = static_cast<int>(xJugador / ANCHO_TERCIO);
    int fila = static_cast<int>(yJugador / ALTO_TERCIO);

    // Seguridad para no exceder los índices de los arreglos si el jugador toca un borde extremo
    col = std::clamp(col, 0, 2);
    fila = std::clamp(fila, 0, 2);

    zonaActualJugador = col + (fila * 3);
}

int AgenteInteligente::razonar() const {
    // El agente busca la zona con el mayor peso
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

void AgenteInteligente::aprender(int zona, float recompensa) {
    // Implementación exacta de la fórmula EWMA: W_nuevo = W_anterior * (1 - alpha) + x * alpha
    // Donde x es la recompensa (1 si dañó, 0 si falló).
    pesosZonas[zona] = pesosZonas[zona] * (1.0f - alpha) + recompensa * alpha;
}

QPointF AgenteInteligente::obtenerCoordenadaPortal(int zona) const {
    // Extraer columna y fila a partir del índice (0-8)
    int col = zona % 3;
    int fila = zona / 3;

    // Calcular el centro exacto de ese tercio de la pantalla
    float xCentro = (col * ANCHO_TERCIO) + (ANCHO_TERCIO / 2.0f);
    float yCentro = (fila * ALTO_TERCIO) + (ALTO_TERCIO / 2.0f);

    return QPointF(xCentro, yCentro);
}
