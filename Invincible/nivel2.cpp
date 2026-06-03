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
    tiempoRestante(90.0f),
    tiempoSpawn(0.0f),
    frecuenciaSpawn(5.0f),
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

    // ─── NUEVO: ARREGLO DE RUTAS DE SPRITES INDIVIDUALES ───
    // Reemplaza los nombres de los archivos por los que tengas en tu carpeta de Sprites.
    // Recuerda que los índices 2 y 5 están reservados para las variantes voladoras.
    QString rutasSprites[6] = {
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Movincihawk_85x85.png",      // Variante Normal 1
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Full-Mask-Mark.png",         // Variante Normal 2
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Capevincible_85x85.png",     // Variante Portal 1 (Voladora)
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Viltrumincible_85x85.png",   // Variante Normal 3
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Hoodvincible_85x85.png",     // Variante Normal 4 (Ejemplo crossover)
        "C:/Users/Andres/OneDrive - Universidad de Antioquia/Escritorio/INFORMATICA_II/Proyecto Final/Sprites/Omni-Mark_85x85.png"         // Variante Portal 2 (Voladora)
    };

    // Crear las 6 variantes de la invasión y dejarlas en espera
    for (int i = 0; i < 6; ++i) {
        if (i == 2 || i == 5) {
            // Las variantes del Agente (Voladoras)
            misSeisVariantes[i] = new VariantePortal(-1000.0f, -1000.0f, 300.0f, 75.0f, 15.0f, 150);
        } else {
            // Las variantes de combate normal (Cuerpo a cuerpo)
            misSeisVariantes[i] = new Variante(-1000.0f, -1000.0f, 500.0f, 75.0f, 10.0f, 100);
        }

        // ─── CAMBIO: ASIGNACIÓN DINÁMICA DEL SPRITE ───
        // Lee el archivo correspondiente desde el arreglo usando el índice actual 'i'
        misSeisVariantes[i]->setPixmap(QPixmap(rutasSprites[i]));

        misSeisVariantes[i]->hide(); // Las ocultamos
        variantes.append(misSeisVariantes[i]);
        escena->addItem(misSeisVariantes[i]);
    }

    // Cargar los elementos gráficos de la barra
    inicializarUI();
}

void Nivel2::actualizarCicloJuego() {
    float dt = 0.016f;

    // 1. Lógica de supervivencia
    tiempoRestante -= dt;
    if (tiempoRestante < 0.0f) tiempoRestante = 0.0f;

    gestionarPortalesEntorno(dt);

    // 2. Temporizador de Variantes Normales (El enjambre cuerpo a cuerpo)
    tiempoSpawn += dt;
    if (tiempoSpawn >= frecuenciaSpawn) {
        spawnVariante();
        tiempoSpawn = 0.0f;
    }

    // 3. Temporizador de Variantes Portal (Voladoras)
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

    // 5. Coordinar IA y Actualizar Enemigos
    QList<Variante*> enjambre;
    int ordenNormales[] = {0, 1, 3, 4}; // Los índices fijos de las variantes cuerpo a cuerpo

    // a. Censar el estado actual del mapa
    for (int idx : ordenNormales) {
        Variante* v = dynamic_cast<Variante*>(misSeisVariantes[idx]);
        if (v && v->isVisible() && v->getVida() > 0.0f) {
            enjambre.append(v);
        }
    }

    int cantidad = enjambre.size();
    if (cantidad > 0) {
        float separacionAngular = (2.0f * 3.14159f) / cantidad; // Reparte el círculo
        bool hayAtacante = false;

        for (int i = 0; i < cantidad; ++i) {
            if (enjambre[i]->getEstado() == Variante::Estado::ATACAR) hayAtacante = true;
            enjambre[i]->setAngulo(i * separacionAngular); // Asigna posición en el cerco
        }

        // b. Otorgar el turno de ataque si nadie lo tiene
        if (!hayAtacante) {
            enjambre[rand() % cantidad]->setEstado(Variante::Estado::ATACAR);
        }
    }

    // c. Actualizar físicas y movimiento
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

                // La variante retrocede después de acertar su golpe
                Variante* vn = dynamic_cast<Variante*>(c);
                if (vn) vn->setEstado(Variante::Estado::RODEAR);

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
    if (portales.isEmpty()) return;

    for (int idx : ordenVoladoras) {
        Enemigo* v = misSeisVariantes[idx];
        VariantePortal* vp = dynamic_cast<VariantePortal*>(v);

        // Condiciones: viva, oculta, y NUNCA lanzada antes (zonaOrigen == -1)
        if (vp && !v->isVisible() && v->getVida() > 0.0f && vp->getZonaOrigen() == -1) {
            int zonaElegida = agente->razonar();
            QPointF posObjetivo = agente->obtenerCoordenadaPortal(zonaElegida);

            // REUTILIZAR PORTALES
            // 1. Elegimos un portal al azar de la lista de portales activos
            int indicePortalAleatorio = rand() % portales.size();
            Portal* portalCuna = portales[indicePortalAleatorio];

            // 2. Posicionamos a la variante exactamente en el centro del portal elegido
            vp->setPosicion(portalCuna->getX(), portalCuna->getY());

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

            // Si el jugador interrumpe el ataque, la variante pierde su turno
            Variante* vn = dynamic_cast<Variante*>(c);
            if (vn) vn->setEstado(Variante::Estado::RODEAR);

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


bool Nivel2::nivelCompletado() const {
    for (int i = 0; i < 6; ++i) {
        if (misSeisVariantes[i]->getVida() > 0.0f)
            return false;
    }
    return true;
}


void Nivel2::verificarVictoria() {
    if (nivelCompletado()) tiempoRestante = 0.0f;
}

void Nivel2::gestionarPortalesEntorno(float dt) {
    // 1. Censar cuántas variantes voladoras siguen vivas en el mapa
    int voladorasVivas = 0;
    for (int i = 0; i < 6; ++i) {
        VariantePortal* vp = dynamic_cast<VariantePortal*>(misSeisVariantes[i]);
        if (vp && vp->getVida() > 0.0f) {
            voladorasVivas++;
        }
    }

    // 2. Definir los límites matemáticos según la cantidad de sobrevivientes
    int minPortales = 0, maxPortales = 0;
    if (voladorasVivas >= 2) {
        minPortales = 5; maxPortales = 7;
    } else if (voladorasVivas == 1) {
        minPortales = 2; maxPortales = 4;
    } else {
        // Si las mataste a ambas, colapsan todos los portales
        for (Portal* p : std::as_const(portales)) {
            escena->removeItem(p);
            delete p;
        }
        portales.clear();
        targetPortales = 0;
        return;
    }

    // Ajuste de seguridad si cambia bruscamente la cantidad de voladoras vivas
    if (targetPortales < minPortales) targetPortales = minPortales;
    if (targetPortales > maxPortales) targetPortales = maxPortales;

    // 3. Fluctuación rítmica (Cambia la cantidad objetivo cada 1.0 segundo)
    tiempoFluctuacionPortales += dt;
    if (tiempoFluctuacionPortales > 1.0f) {
        tiempoFluctuacionPortales = 0.0f;

        if (creciendoPortales) {
            targetPortales++;
            if (targetPortales >= maxPortales) creciendoPortales = false;
        } else {
            targetPortales--;
            if (targetPortales <= minPortales) creciendoPortales = true;
        }
    }

    // 4. Sincronizar el escenario visual con el target calculado
    // ABRIR NUEVOS PORTALES
    while (portales.size() < targetPortales) {

        // 1. Calculamos una fase aleatoria (en radianes) para que los portales
        // no aparezcan amontonados, sino repartidos por toda la pista elíptica.
        float faseAleatoria = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;

        // 2. Usamos el constructor Orbital: (centroX, centroY, radio, velocidad, fase)
        Portal* nuevoPortal = new Portal(350.0f, 250.0f, 290.0f, 1.0f, faseAleatoria);

        escena->addItem(nuevoPortal);
        portales.append(nuevoPortal);
    }

    // CERRAR PORTALES SOBRANTES
    while (portales.size() > targetPortales && !portales.isEmpty()) {
        // Elimina el portal más antiguo de la lista
        Portal* pViejo = portales.takeFirst();
        escena->removeItem(pViejo);
        delete pViejo;
    }
}
