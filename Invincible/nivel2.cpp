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
    tiempoRestante(100.0f),
    tiempoSpawn(0.0f),
    frecuenciaSpawn(10.0f),
    tiempoSpawnVoladoras(0.0f),
    frecuenciaSpawnVoladoras(1.0f) // Voladoras cada 1s
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
            misSeisVariantes[i] = new VariantePortal(-1000.0f, -1000.0f, 300.0f, 75.0f, 15.0f, 150); // (float x, float y, float vida, float masa, float danio, uint16_t puntos)
            misSeisVariantes[i]->setPixmap(QPixmap("C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Capevincible_85x85.png"));
        } else { // Las variantes de combate normal
            misSeisVariantes[i] = new Variante(-1000.0f, -1000.0f, 500.0f, 75.0f, 10.0f, 100);
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
    float dt = 0.016f;

    // 1. Tiempo de supervivencia
    tiempoRestante -= dt;
    if (tiempoRestante < 0.0f) tiempoRestante = 0.0f;

    // 2. Spawn de normales (cada 2.5s)
    tiempoSpawn += dt;
    if (tiempoSpawn >= frecuenciaSpawn) {
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    // 3. Spawn de voladoras (cada 1.2s)
    tiempoSpawnVoladoras += dt;
    if (tiempoSpawnVoladoras >= frecuenciaSpawnVoladoras) {
        spawnVoladora();
        tiempoSpawnVoladoras = 0.0f;
    }

    // 4. Actualizar Jugador
    if (jugador && jugador->isActivo()) {
        jugador->update(dt);
        agente->percibir(jugador->getX(), jugador->getY());
    }

    // 5. Actualizar Enemigos
    for (Enemigo* c : std::as_const(variantes)) {
        if (c->isActivo() && jugador) {
            c->update(dt, *jugador);

            if (c->opacity() < 1.0f) {
                c->setOpacity(c->opacity() + dt * 4.0f);
                if (c->opacity() > 1.0f) c->setOpacity(1.0f);
            }
        }
    }

    // 6. Actualizar Portales
    for (Portal* p : std::as_const(portales)) {
        if (p->isActivo()) {
            p->updateConPhysics(dt, *physics);
        }
    }

    verificarColisiones();
    limpiarInactivos();
    limpiarPortales();
    verificarVictoria();
    actualizarUI();
}

void Nivel2::verificarColisiones() {
    if (!jugador || !jugador->isActivo()) return;

    float xJ = jugador->getX();
    float yJ = jugador->getY();

    for (Enemigo* c : std::as_const(variantes)) {
        if (!c->isActivo() || !c->isVisible()) continue;

        // Colisión AABB
        if (std::abs(xJ - c->getX()) < 60.0f && std::abs(yJ - c->getY()) < 60.0f) {

            // APRENDIZAJE
            VariantePortal* vp = dynamic_cast<VariantePortal*>(c);
            if (vp && !vp->getImpacto()) { //Solo si NO había impactado antes
                vp->setImpacto(true);
                agente->aprender(vp->getZonaOrigen(), 1.0f);
            }

            if (!jugador->isInvulnerable()) {
                jugador->recibirDanio(c->getDanio());

                float dx = xJ - c->getX();
                float dy = yJ - c->getY();
                float magnitud = std::sqrt(dx * dx + dy * dy);

                if (magnitud > 0.0f) {
                    float dirX = dx / magnitud;
                    float dirY = dy / magnitud;
                    float fuerzaEmpuje = 40.0f;
                    float ratioMasa = c->getMasa() / jugador->getMasa();
                    float empujeFinal = fuerzaEmpuje * ratioMasa;

                    jugador->setPosicion(xJ + (dirX * empujeFinal), yJ + (dirY * empujeFinal));
                }
            }
        }
    }
}

void Nivel2::spawnVariante() {
    // Orden fijo de aparición de las 4 normales
    int ordenNormales[] = {0, 1, 3, 4};
    for (int idx : ordenNormales) {
        Enemigo* v = misSeisVariantes[idx];

        // Solo si está viva y nunca ha entrado a la pantalla (está oculta)
        if (!v->isVisible() && v->getVida() > 0.0f) {
            float xAleatorio = 100.0f + (rand() % 600);
            float yAleatorio = 100.0f + (rand() % 400);
            v->setPosicion(xAleatorio, yAleatorio);
            v->show();
            return; // Solo aparece una por cada vez que se cumple el timer
        }
    }
}

void Nivel2::spawnVoladora() {
    int ordenVoladoras[] = {2, 5};

    for (int idx : ordenVoladoras) {
        Enemigo* v = misSeisVariantes[idx];
        VariantePortal* vp = dynamic_cast<VariantePortal*>(v);

        // Condiciones: viva, oculta, y NUNCA lanzada antes (zonaOrigen == -1)
        if (vp && !v->isVisible() && v->getVida() > 0.0f && vp->getZonaOrigen() == -1) {
            int zonaElegida = agente->razonar();
            QPointF posObjetivo = agente->obtenerCoordenadaPortal(zonaElegida);

            // Crear portal orbital
            float faseAleatoria = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
            Portal* nuevoPortal = new Portal(350.0f, 250.0f, 290.0f, 1.0f, faseAleatoria);
            portales.append(nuevoPortal);
            escena->addItem(nuevoPortal);

            // Posicionar voladora en el portal
            vp->setPosicion(nuevoPortal->getX(), nuevoPortal->getY());
            vp->setZonaOrigen(zonaElegida);   // Marca que ya fue lanzada
            vp->setImpacto(false);            // Reset para aprendizaje

            // Disparar hacia la zona predicha por el agente
            float dx = posObjetivo.x() - vp->getX();
            float dy = posObjetivo.y() - vp->getY();
            float magnitud = std::sqrt(dx*dx + dy*dy);

            if (magnitud > 0.0f) {
                vp->setVelocidad((dx / magnitud) * 370.0f, (dy / magnitud) * 370.0f);
            }

            vp->show();
            return; // Solo una por tick de timer
        }
    }
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

            // FÍSICA DE EMPUJE A LA VARIANTE
            // 1. Calcular el vector de dirección desde el jugador hacia la variante
            float dx = c->getX() - xJ;
            float dy = c->getY() - yJ;
            float magnitud = std::sqrt(dx * dx + dy * dy);

            if (magnitud > 0.0f) {
                // Vector normalizado (dirección pura)
                float dirX = dx / magnitud;
                float dirY = dy / magnitud;

                // 2. Definir la fuerza base del empuje
                float fuerzaEmpuje = 50.0f;

                // Si es el golpe cargado de la pasiva "Fortachón" o un combo, empuja el doble
                if (danio > 20.0f) {
                    fuerzaEmpuje *= 2.5f;
                }

                // 3. Aplicar tu ecuación de conservación de momento lineal por masas
                float ratioMasa = jugador->getMasa() / c->getMasa();
                float empujeFinal = fuerzaEmpuje * ratioMasa;

                // 4. Desplazar a la variante
                c->setPosicion(c->getX() + (dirX * empujeFinal), c->getY() + (dirY * empujeFinal));
            }
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

        // Si es voladora, está viva, y acaba de terminar su vuelo (velX == 0)
        if (vp && vp->getVida() > 0.0f && vp->getVelX() == 0.0f && !vp->isVisible()) {

            // Verificamos que venía de un vuelo real
            if (vp->getZonaOrigen() != -1) {
                if (!vp->getImpacto()) {
                    agente->aprender(vp->getZonaOrigen(), 0.0f); // Falló, aprende 0.0
                }

                vp->setImpacto(true);
                vp->setZonaOrigen(-1);
            }
        }
    }
}

void Nivel2::limpiarPortales() {
    // Verificamos si alguna de las dos voladoras (índices 2 o 5) sigue cruzando la pantalla
    bool hayVoladoras = misSeisVariantes[2]->isVisible() || misSeisVariantes[5]->isVisible();

    // Si ya se ocultaron, limpiamos todos los portales residuales
    if (!hayVoladoras) {
        for (Portal* p : std::as_const(portales)) {
            escena->removeItem(p);
            delete p;
        }
        portales.clear();
    }
}

bool Nivel2::nivelCompletado() const {
    for (int i = 0; i < 6; ++i) {
        if (misSeisVariantes[i]->getVida() > 0.0f)
            return false;
    }
    return true;
}


void Nivel2::verificarVictoria() {
    //if (nivelCompletado()) tiempoRestante = 0.0f;
}
