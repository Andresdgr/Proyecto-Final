#ifndef AGENTEINTELIGENTE_H
#define AGENTEINTELIGENTE_H

#include <QPointF>

class AgenteInteligente {
public:
    AgenteInteligente();

    // 1. Percepción: Actualiza la posición del jugador en la grilla
    void percibir(float xJugador, float yJugador);

    // 2. Razonamiento: Decide en qué zona de las 9 es mejor atacar
    int razonar() const;

    // 3. Aprendizaje: Actualiza los pesos con el modelo EWMA
    // recompensa (x): 1.0f si dañó al jugador, 0.0f si el jugador escapó
    void aprender(int zona, float recompensa);

    // 4. Acción (Auxiliar): Obtiene las coordenadas (x,y) exactas del centro de una zona
    QPointF obtenerCoordenadaPortal(int zona) const;

private:
    // Vector de pesos para las 9 zonas
    float pesosZonas[9];

    // Tasa de Aprendizaje (alpha): 0 a 1, define qué tan rápido olvida el agente
    float alpha;

    int zonaActualJugador;

    // Dimensiones estáticas para la pantalla de 800x600
    static constexpr float ANCHO_PANTALLA = 800.0f;
    static constexpr float ALTO_PANTALLA = 600.0f;
    static constexpr float ANCHO_TERCIO = 800.0f / 3.0f;
    static constexpr float ALTO_TERCIO = 600.0f / 3.0f;
};

#endif // AGENTEINTELIGENTE_H
