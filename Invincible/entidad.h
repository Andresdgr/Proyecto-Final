#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QString>

class Entidad : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
protected:
    double posX;
    double posY;
    double velX;
    double velY;

public:
    Entidad(double x, double y, QGraphicsItem* parent = nullptr);
    virtual ~Entidad() {} // Destructor virtual para correcta destrucción en cascada

    virtual void actualizarPosicion() = 0;
    virtual void cargarSprite(const QString& rutaSprite) = 0;

    double getPosX() const { return posX; }
    double getPosY() const { return posY; }
    void setVelocidad(double vx, double vy) { velX = vx; velY = vy; }
};

#endif // ENTIDAD_H
