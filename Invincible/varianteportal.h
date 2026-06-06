#ifndef VARIANTEPORTAL_H
#define VARIANTEPORTAL_H

#include "enemigo.h"
#include <QPointF>

class VariantePortal : public Enemigo {
public:
    VariantePortal(float x, float y, float vida, float masa,
                   float danio, uint16_t puntos);

    void update(float dt) override;
    void update(float dt, const Jugador& jugador) override;
    void moverse(float dt) override;

    void setZonaOrigen(int zona)    { zonaOrigen = zona;      }
    int  getZonaOrigen() const      { return zonaOrigen;      }
    void setImpacto(bool impacto)   { impactoAlJugador = impacto; }
    bool getImpacto() const         { return impactoAlJugador; }

    // Agente inteligente compartido (static)
    static void    percibir(float xJugador, float yJugador);
    static int     razonar();
    static void    aprender(int zona, float recompensa);
    static QPointF obtenerCoordenadaPortal(int zona);

private:
    int  zonaOrigen        = -1;
    bool impactoAlJugador  = false;

    static float pesosZonas[9];
    static int   zonaActualJugador;
    static constexpr float alpha       = 0.3f;
    static constexpr float ANCHO_TERCIO = 800.0f / 3.0f;
    static constexpr float ALTO_TERCIO  = 600.0f / 3.0f;
};

#endif // VARIANTEPORTAL_H
