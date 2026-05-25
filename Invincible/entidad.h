#ifndef ENTIDAD_H
#define ENTIDAD_H

class Entidad {

public: //Metodos Todo lo que es accesible desde cualquier parte del programa.

    Entidad(float x, float y, float vida, float masa);

    virtual ~Entidad();

    virtual void update(float dt) = 0;


    virtual void recibirDanio(float cantidad);

    float getX()          const;
    float getY()          const;
    float getVelX()       const;
    float getVelY()       const;
    float getVida()       const;
    float getVidaMaxima() const;
    float getMasa()       const;
    bool  isActivo()      const;

protected://el exterior solo puede leerlos mediante getters const

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
