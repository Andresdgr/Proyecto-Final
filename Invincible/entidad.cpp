#include "entidad.h"


Entidad::Entidad(double x, double y, QGraphicsItem* parent)
    : QObject(), QGraphicsPixmapItem(parent), posX(x), posY(y), velX(0), velY(0) {

    // Posicionar la entidad en la escena de Qt al momento de crearla
    setPos(posX, posY);
}
