#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <QGraphicsPixmapItem>

class Entidad : public QGraphicsPixmapItem { // 1. Añadir herencia gráfica

public:
    // 2. Se añade un puntero opcional a QGraphicsItem por convención de Qt
    Entidad(float x, float y, float vida, float masa, QGraphicsItem* parent = nullptr);
    virtual ~Entidad();

    virtual void update(float dt) = 0;
    virtual void recibirDanio(float cantidad);

    // ── Getters ─────────────────────────────────────────────────────
    float getX()          const;
    float getY()          const;
    float getVelX()       const;
    float getVelY()       const;
    float getVida()       const;
    float getVidaMaxima() const;
    float getMasa()       const;
    bool  isActivo()      const;

    // ── Setters ─────────────────────────────────────────────────────
    // 3. Nuevos métodos para sincronizar las matemáticas con los gráficos
    void setPosicion(float nuevoX, float nuevoY);
    void setVelocidad(float vX, float vY);

protected:
    float x;
    float y;

    float velX;
    float velY;

    float vida;
    float vidaMaxima;

    float masa;

    bool activo;
};

#endif
