#include "nivel2.h"
#include "jugador.h"
#include "variante.h"
#include "varianteportal.h"
#include <QGraphicsPixmapItem>
#include <utility>
#include "physicsengine.h"
#include "difficultyconfig.h"
#include <cmath> // Para std::abs
#include <cstdlib> // Para rand()

Nivel2::Nivel2(QGraphicsScene* escenaCompartida)
    : Nivel(escenaCompartida),
    tiempoRestante(60.0f),
    tiempoSpawn(0.0f),
    frecuenciaSpawn(4.0f),
    indiceSecuencia(0)
{
    physics = new PhysicsEngine(DifficultyConfig::normal());
    agente = new AgenteInteligente();
}

Nivel2::~Nivel2() {
    qDeleteAll(variantes);
    variantes.clear();

    qDeleteAll(portales);
    portales.clear();

    // 'jugador' y 'gameTimer' se eliminan en el destructor de la clase base Nivel.
    delete physics;
    delete agente;
}

void Nivel2::inicializarEscenario() {
    // 1. Cargar fondo del nivel
    QGraphicsPixmapItem* fondo = new QGraphicsPixmapItem(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Nivel_2.png"));
    escena->addItem(fondo);

    // Forzar los límites lógicos de la escena
    escena->setSceneRect(0, 0, 800, 600);

    // 2. Instanciar jugador (el puntero 'jugador' está protegido en la clase Nivel)
    jugador = new Jugador(400.0f, 300.0f);
    escena->addItem(jugador);

    // Crear las 6 variantes de la invasión y dejarlas en espera
    for (int i = 0; i < 6; ++i) {
        if (i == 2 || i == 5) { // Las variantes del Agente
            misSeisVariantes[i] = new VariantePortal(-1000.0f, -1000.0f, 100.0f, 75.0f, 15.0f, 150);
            misSeisVariantes[i]->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Capevincible_85x85.png"));
        } else { // Las variantes de combate normal
            misSeisVariantes[i] = new Variante(-1000.0f, -1000.0f, 100.0f, 75.0f, 10.0f, 100);
            misSeisVariantes[i]->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Viltrumincible_85x85.png"));
        }

        misSeisVariantes[i]->hide(); // Las ocultamos
        variantes.append(misSeisVariantes[i]);
        escena->addItem(misSeisVariantes[i]);
    }

    // Cargar los elementos gráficos de la barra
    inicializarUI();
}

void Nivel2::actualizarCicloJuego() {
    // El QTimer en nivel.cpp corre a 16ms, por lo tanto dt es aprox 0.016 segundos.
    float dt = 0.016f;

    // 1. Lógica de supervivencia
    tiempoRestante -= dt;
    if (tiempoRestante < 0.0f) tiempoRestante = 0.0f;

    tiempoSpawn += dt;
    if (tiempoSpawn >= frecuenciaSpawn) {
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    // 2. Actualizar Jugador
    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
        // El agente registra en cuál de las 9 zonas está el jugador
        agente->percibir(jugador->getX(), jugador->getY());
    }

    // 3. Actualizar Enemigos
    for (Enemigo* c : std::as_const(variantes)) {
        if (c->isActivo() && jugador) {
            c->update(dt, *jugador);

            // Si el clon fue golpeado, recupera su opacidad gradualmente
            if (c->opacity() < 1.0f) {
                c->setOpacity(c->opacity() + dt * 4.0f); // Retorna al estado sólido (1.0)
                if (c->opacity() > 1.0f) c->setOpacity(1.0f);
            }
        }
    }

    // 4. Actualizar Portales
    for (Portal* p : std::as_const(portales)) {
        if (p->isActivo()) {
            // Requiere inyectar PhysicsEngine si se maneja aquí
            p->updateConPhysics(dt, *physics);
        }
    }

    verificarColisiones();
    limpiarInactivos();
    actualizarUI();
}

void Nivel2::verificarColisiones() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    for (Enemigo* c : std::as_const(variantes)) {
        if (!c->isActivo() || !c->isVisible()) continue;

        // APRENDIZAJE: Si es una VariantePortal, el agente recibe recompensa de 1
        VariantePortal* vp = dynamic_cast<VariantePortal*>(c);
        if (vp) {
            vp->setImpacto(true); // Marcamos que cumplió su objetivo
            agente->aprender(vp->getZonaOrigen(), 1.0f);
        }
        // Lógica AABB adaptada de tu GameEngine
        if (std::abs(xJ - c->getX()) < 60.0f && std::abs(yJ - c->getY()) < 60.0f) {
            jugador->recibirDanio(c->getDanio());
            if (jugador->getDanioActual() > 0.0f) {
                c->recibirDanio(jugador->getDanioActual());
            }
        }
    }
}

void Nivel2::spawnVariante() {
    // 1. Contar estrictamente qué tenemos vivo Y visible en el mapa
    int normalesVivas = 0;
    int voladorasVivas = 0;

    for (Enemigo* e : std::as_const(variantes)) {
        // CORRECCIÓN: Como ahora las 6 están creadas desde el inicio (solo que ocultas),
        // debemos contar únicamente las que están actualmente peleando en pantalla.
        if (e->isVisible() && e->getVida() > 0.0f) {
            if (dynamic_cast<VariantePortal*>(e)) {
                voladorasVivas++;
            } else {
                normalesVivas++;
            }
        }
    }

    // 2. Determinar qué toca spawnear y bloquear si ya estamos al límite
    bool esTurnoVoladora = (indiceSecuencia == 2 || indiceSecuencia == 5);

    if (esTurnoVoladora && voladorasVivas >= 2) return; // Pausa el spawn de portales
    if (!esTurnoVoladora && normalesVivas >= 4) return; // Pausa el spawn de normales

    // 3. Rescatar la variante pre-creada que corresponde a este turno
    Enemigo* varianteActual = misSeisVariantes[indiceSecuencia];

    // Si la variante actual ya fue derrotada (vida en 0) o ya está peleando en pantalla,
    if (varianteActual->getVida() <= 0.0f || varianteActual->isVisible()) {
        indiceSecuencia = (indiceSecuencia + 1) % 6;
        return;
    }

    // 4. Configurar el comportamiento según el tipo
    if (esTurnoVoladora) {
        // --- EL AGENTE EN ACCIÓN ---
        int zonaElegida = agente->razonar();

        // Guardamos el centro de la zona...
        QPointF posObjetivo = agente->obtenerCoordenadaPortal(zonaElegida);

        float faseAleatoria = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
        Portal* nuevoPortal = new Portal(400.0f, 300.0f, 290.0f, 1.0f, faseAleatoria);
        portales.append(nuevoPortal);
        escena->addItem(nuevoPortal);

        // Extraemos la variante voladora desde nuestro arreglo
        VariantePortal* varianteVoladora = dynamic_cast<VariantePortal*>(varianteActual);

        // La posicionamos en el portal
        varianteVoladora->setPosicion(nuevoPortal->getX(), nuevoPortal->getY());
        varianteVoladora->setZonaOrigen(zonaElegida);
        varianteVoladora->setImpacto(false); // Reseteamos el flag de aprendizaje

        // ...Y HACEMOS QUE LA VARIANTE DISPARE HACIA ESA ZONA PREDICHA, NO AL JUGADOR
        float dx = posObjetivo.x() - varianteVoladora->getX();
        float dy = posObjetivo.y() - varianteVoladora->getY();
        float magnitud = std::sqrt(dx*dx + dy*dy);

        if (magnitud > 0.0f) {
            varianteVoladora->setVelocidad((dx / magnitud) * 250.0f, (dy / magnitud) * 250.0f);
        }

    } else {
        // Spawn Variante Normal
        float xAleatorio = 100.0f + (rand() % 600);
        float yAleatorio = 100.0f + (rand() % 400);

        // Simplemente la posicionamos en las coordenadas aleatorias
        varianteActual->setPosicion(xAleatorio, yAleatorio);
    }

    // 5. Mostrar la variante en la interfaz
    varianteActual->show();


    // Avanzar la secuencia de invasión (0 a 5)
    indiceSecuencia = (indiceSecuencia + 1) % 6;
}

void Nivel2::verificarAtaqueJugador() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();
    float danio = jugador->getDanioActual();

    for (Enemigo* c : std::as_const(variantes)) {
        if (!c->isActivo() || !c->isVisible()) continue;

        // Determinar proximidad en un rango de ataque (ej. 80 píxeles)
        if (std::abs(xJ - c->getX()) < 80.0f && std::abs(yJ - c->getY()) < 80.0f) {
            c->recibirDanio(danio);

            // VISUALIZACIÓN DEL DAÑO:
            c->setOpacity(0.3f);
        }
    }
}


void Nivel2::inicializarUI() {
    // 1. Barra del Jugador (Superior Izquierda)
    barraFondoJugador = new QGraphicsRectItem(10, 10, 200, 20);
    barraFondoJugador->setBrush(Qt::darkRed);
    escena->addItem(barraFondoJugador);

    barraVidaJugador = new QGraphicsRectItem(10, 10, 200, 20);
    barraVidaJugador->setBrush(Qt::green);
    escena->addItem(barraVidaJugador);

    // 2. Timer (Centro Superior)
    textoTiempo = new QGraphicsTextItem();
    textoTiempo->setDefaultTextColor(Qt::white);
    textoTiempo->setFont(QFont("Arial", 22, QFont::Bold));
    textoTiempo->setPos(370, 5); // Centrado aprox
    escena->addItem(textoTiempo);

    // 3. Grid de Variantes (Superior Derecha: 2 filas x 3 columnas)
    float inicioX = 600.0f;
    float inicioY = 10.0f;
    float anchoBarra = 50.0f;
    float altoBarra = 10.0f;
    float margenX = 15.0f; // Espacio horizontal entre barras
    float margenY = 15.0f; // Espacio vertical entre filas

    for (int i = 0; i < 6; ++i) {
        int columna = i % 3;
        int fila = i / 3;
        float x = inicioX + columna * (anchoBarra + margenX);
        float y = inicioY + fila * (altoBarra + margenY);

        barrasFondoVariantes[i] = new QGraphicsRectItem(x, y, anchoBarra, altoBarra);
        barrasFondoVariantes[i]->setBrush(Qt::black);
        barrasFondoVariantes[i]->hide(); // Ocultas por defecto
        escena->addItem(barrasFondoVariantes[i]);

        barrasVidaVariantes[i] = new QGraphicsRectItem(x, y, anchoBarra, altoBarra);
        barrasVidaVariantes[i]->setBrush(Qt::yellow);
        barrasVidaVariantes[i]->hide();
        escena->addItem(barrasVidaVariantes[i]);
    }
}

void Nivel2::actualizarUI() {
    // 1. Actualizar Timer
    if (textoTiempo) {
        int t = static_cast<int>(tiempoRestante);
        // Formato con ceros a la izquierda (ej. "09")
        textoTiempo->setPlainText(QString("%1").arg(t, 2, 10, QChar('0')));
    }

    // 2. Actualizar Barra del Jugador
    if (jugador) {
        float porcentajeVida = jugador->getVida() / jugador->getVidaMaxima();
        if (porcentajeVida < 0.0f) porcentajeVida = 0.0f;

        // Mantener las mismas coordenadas (10, 10) pero escalar el ancho (200 máximo)
        barraVidaJugador->setRect(10, 10, 200.0f * porcentajeVida, 20);
    }

    // 3. Actualizar Grid de Variantes Dinámicamente
    for (int i = 0; i < 6; ++i) {
        Enemigo* v = misSeisVariantes[i];

        // Mantener las barras a la vista
        barrasFondoVariantes[i]->show();
        barrasVidaVariantes[i]->show();

        float porcentajeEnemigo = v->getVida() / v->getVidaMaxima();
        if (porcentajeEnemigo < 0.0f) porcentajeEnemigo = 0.0f;

        QRectF rectBase = barrasFondoVariantes[i]->rect();
        barrasVidaVariantes[i]->setRect(rectBase.x(), rectBase.y(), 50.0f * porcentajeEnemigo, rectBase.height());
    }
}

void Nivel2::limpiarInactivos() {
    for (int i = 0; i < 6; ++i) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(misSeisVariantes[i]);

        // Si es voladora, está viva, y se detuvo (velX == 0) tras salir de pantalla sin impactar
        if (vp && vp->getVida() > 0.0f && vp->getVelX() == 0.0f && !vp->isVisible()) {
            if (!vp->getImpacto()) {
                agente->aprender(vp->getZonaOrigen(), 0.0f); // Recompensa negativa
                vp->setImpacto(true); // Lo marcamos true para que no aprenda el mismo fallo dos veces
            }
        }
    }
}
