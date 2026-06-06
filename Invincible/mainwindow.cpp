#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QMouseEvent>
#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QTransform>
#include <QCoreApplication>
#include <cmath>
#include "varianteportal.h"
#include "variante.h"

// ── Constructor ─────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , escena(nullptr)
    , vista(nullptr)
    , timerLoop(nullptr)
    , config(DifficultyConfig::normal())
    , engine(nullptr)
    , spriteJugador(nullptr)
    , spriteLevy(nullptr)
    , spritePortal(nullptr)
    , itemFondo(nullptr)
    , barraVidaJugador(nullptr)
    , barraVidaLevy(nullptr)
    , fondoVidaLevy(nullptr)
    , labelLevy(nullptr)
    , textoPuntos(nullptr)
    , textoTiempo(nullptr)
    , textoFinJuego(nullptr)
    , textoReiniciar(nullptr)
    , textoPuntajeFinal(nullptr)
    , btnPausaFondo(nullptr)
    , btnPausaTexto(nullptr)
    , enMenu(true)
    , mostrandoGolpe(false)
    , contadorGolpe(0)
    , mirandoDerecha(true)
    , levyGolpeando(false)
    , contadorLevyGolpe(0)
    , musicaFondo(nullptr)
    , audioFondo(nullptr)
    , efectoGolpe(nullptr)
    , efectoVictoria(nullptr)
    , efectoDerrota(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("Invincible Kickboxer");
    setFixedSize(800, 600);

    engine = new GameEngine(config);

    inicializarEscena();
    inicializarHUD();

    timerLoop = new QTimer(this);
    connect(timerLoop, &QTimer::timeout,
            this,      &MainWindow::onUpdate);

    reloj.start();

    // ── Inicializar audio ─────────────────────────────────────
    QString rutaBase = QCoreApplication::applicationDirPath() + "/soundtrack/";

    musicaFondo = new QMediaPlayer(this);
    audioFondo  = new QAudioOutput(this);
    musicaFondo->setAudioOutput(audioFondo);
    audioFondo->setVolume(0.4f);
    musicaFondo->setLoops(QMediaPlayer::Infinite);

    efectoGolpe = new QSoundEffect(this);
    efectoGolpe->setSource(QUrl::fromLocalFile(
        rutaBase + "golpe.wav"));
    efectoGolpe->setVolume(0.8f);

    efectoVictoria = new QSoundEffect(this);
    efectoVictoria->setSource(QUrl::fromLocalFile(
        rutaBase + "win.wav"));
    efectoVictoria->setVolume(1.0f);

    efectoDerrota = new QSoundEffect(this);
    efectoDerrota->setSource(QUrl::fromLocalFile(
        rutaBase + "lose.wav"));
    efectoDerrota->setVolume(1.0f);

    nivelInicioActual = 1;
    juegoPausado = false;
    enSubmenuDesdePausa = false;
    dibujarMenuPrincipal();
    cambiarMusica(0);
    setFocus();
}
// ── Destructor ───────────────────────────────────────────────────
MainWindow::~MainWindow()
{
    delete engine;
    delete ui;
}

// ── cambiarMusica ────────────────────────────────────────────────
void MainWindow::cambiarMusica(int nivel)
{
    musicaFondo->stop();

    QString rutaBase = QCoreApplication::applicationDirPath() + "/soundtrack/";

    if (nivel == 0 || nivel == 1) {
        musicaFondo->setSource(QUrl::fromLocalFile(
            rutaBase + "Invincible Theme by John Paesano [gF73gPQQsR4].wav"));
    } else if (nivel == 2) {
        musicaFondo->setSource(QUrl::fromLocalFile(
            rutaBase + "Tom_tom.wav"));
    }

    musicaFondo->play();
}

// ── cambiarFondo ─────────────────────────────────────────────────
void MainWindow::cambiarFondo(int nivel)
{
    if (itemFondo) {
        escena->removeItem(itemFondo);
        delete itemFondo;
        itemFondo = nullptr;
    }

    QPixmap pixFondo;
    if (nivel == 1) {
        pixFondo = QPixmap(":/sprites/assets/sprites/Escenario.png");
    } else {
        pixFondo = QPixmap(":/sprites/Sprites/Nivel_2.png");
    }

    pixFondo = pixFondo.scaled(800, 600,
                               Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);

    itemFondo = new QGraphicsPixmapItem(pixFondo);
    itemFondo->setPos(0, 0);
    itemFondo->setZValue(-1);
    escena->addItem(itemFondo);
}

// ── inicializarEscena ────────────────────────────────────────────
void MainWindow::inicializarEscena()
{
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 800, 600);

    vista = new QGraphicsView(escena, this);
    vista->setGeometry(0, 0, 800, 600);
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setRenderHint(QPainter::Antialiasing);
    vista->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    vista->setFocusPolicy(Qt::NoFocus);

    cambiarFondo(1);

    // ── Sprite jugador ────────────────────────────────────────
    spriteJugador = new QGraphicsPixmapItem();

    pixJugadorNormal = QPixmap(":/sprites/assets/sprites/Invincible.png");
    pixJugadorNormal = pixJugadorNormal.scaled(60, 80,
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);

    pixJugadorNormalIzq = pixJugadorNormal.transformed(
        QTransform().scale(-1, 1));

    pixJugadorGolpe = QPixmap(":/sprites/assets/sprites/invinsibleGolpeSinFondo.png");
    pixJugadorGolpe = pixJugadorGolpe.scaled(80, 80,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);

    pixJugadorGolpeIzq = pixJugadorGolpe.transformed(
        QTransform().scale(-1, 1));

    spriteJugador->setPixmap(pixJugadorNormal);
    spriteJugador->setPos(400, 300);
    spriteJugador->setZValue(1);
    escena->addItem(spriteJugador);

    // ── Sprite Levy ───────────────────────────────────────────
    spriteLevy = new QGraphicsPixmapItem();

    pixLevyNormal = QPixmap(":/sprites/assets/sprites/Amstrongr.png");
    pixLevyNormal = pixLevyNormal.scaled(70, 90,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

    pixLevyNormalIzq = pixLevyNormal.transformed(
        QTransform().scale(-1, 1));

    pixLevyGolpe = QPixmap(":/sprites/assets/sprites/levi_golpe.png");
    pixLevyGolpe = pixLevyGolpe.scaled(70, 90,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);

    pixLevyGolpeIzq = pixLevyGolpe.transformed(
        QTransform().scale(-1, 1));

    spriteLevy->setPixmap(pixLevyNormal);
    spriteLevy->setPos(600, 300);
    spriteLevy->setZValue(1);
    escena->addItem(spriteLevy);

    // ── Sprite Portal ─────────────────────────────────────────
    spritePortal = new QGraphicsPixmapItem();
    QPixmap pixPortal(":/sprites/assets/sprites/Portal.png");
    pixPortal = pixPortal.scaled(80, 80,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
    spritePortal->setPixmap(pixPortal);
    spritePortal->setPos(500, 200);
    spritePortal->setZValue(1);
    escena->addItem(spritePortal);
}

// ── inicializarHUD ───────────────────────────────────────────────
void MainWindow::inicializarHUD()
{
    QGraphicsRectItem* fondoVidaJ = new QGraphicsRectItem(10, 515, 150, 18);
    fondoVidaJ->setBrush(QBrush(QColor(80, 80, 80)));
    fondoVidaJ->setPen(Qt::NoPen);
    escena->addItem(fondoVidaJ);

    barraVidaJugador = new QGraphicsRectItem(10, 515, 150, 18);
    barraVidaJugador->setBrush(QBrush(QColor(0, 200, 100)));
    barraVidaJugador->setPen(Qt::NoPen);
    escena->addItem(barraVidaJugador);

    QGraphicsTextItem* labelJ = new QGraphicsTextItem("Jugador");
    labelJ->setDefaultTextColor(Qt::white);
    labelJ->setFont(QFont("Arial", 8));
    labelJ->setPos(10, 535);
    escena->addItem(labelJ);

    fondoVidaLevy = new QGraphicsRectItem(620, 515, 150, 18);
    fondoVidaLevy->setBrush(QBrush(QColor(80, 80, 80)));
    fondoVidaLevy->setPen(Qt::NoPen);
    escena->addItem(fondoVidaLevy);

    barraVidaLevy = new QGraphicsRectItem(620, 515, 150, 18);
    barraVidaLevy->setBrush(QBrush(QColor(200, 50, 50)));
    barraVidaLevy->setPen(Qt::NoPen);
    escena->addItem(barraVidaLevy);

    labelLevy = new QGraphicsTextItem("Angstrom Levy");
    labelLevy->setDefaultTextColor(Qt::white);
    labelLevy->setFont(QFont("Arial", 8));
    labelLevy->setPos(620, 535);
    escena->addItem(labelLevy);

    textoPuntos = new QGraphicsTextItem("Puntos: 0");
    textoPuntos->setDefaultTextColor(Qt::yellow);
    textoPuntos->setFont(QFont("Arial", 11, QFont::Bold));
    textoPuntos->setPos(340, 512);
    escena->addItem(textoPuntos);

    textoTiempo = new QGraphicsTextItem("");
    textoTiempo->setDefaultTextColor(Qt::cyan);
    textoTiempo->setFont(QFont("Arial", 11, QFont::Bold));
    textoTiempo->setPos(340, 530);
    escena->addItem(textoTiempo);
    crearBotonPausa();
}

// ── onUpdate ─────────────────────────────────────────────────────
void MainWindow::onUpdate()
{
    float dt = reloj.elapsed() / 1000.0f;
    reloj.restart();

    if (dt > 0.05f) dt = 0.05f;

    procesarInput();
    engine->update(dt);
    sincronizarSprites();
    actualizarHUD();
    escena->update();

    GameState::EstadoPartida est = engine->getEstado().getEstado();
    if (est == GameState::EstadoPartida::VICTORIA) {
        mostrarPantallaFin(true);
    } else if (est == GameState::EstadoPartida::DERROTA) {
        mostrarPantallaFin(false);
    }
}

// ── procesarInput ────────────────────────────────────────────────
void MainWindow::procesarInput()
{
    Jugador* jugador = engine->getJugador();
    if (!jugador || !jugador->isActivo()) return;

    bool moviendoX = false;
    bool moviendoY = false;

    if (teclasPresionadas.contains(Qt::Key_Left) ||
        teclasPresionadas.contains(Qt::Key_A)) {
        jugador->moverX(-1.0f);
        moviendoX      = true;
        mirandoDerecha = false;
        if (!mostrandoGolpe)
            spriteJugador->setPixmap(pixJugadorNormalIzq);
    } else if (teclasPresionadas.contains(Qt::Key_Right) ||
               teclasPresionadas.contains(Qt::Key_D)) {
        jugador->moverX(1.0f);
        moviendoX      = true;
        mirandoDerecha = true;
        if (!mostrandoGolpe)
            spriteJugador->setPixmap(pixJugadorNormal);
    }

    if (teclasPresionadas.contains(Qt::Key_Up) ||
        teclasPresionadas.contains(Qt::Key_W)) {
        jugador->moverY(-1.0f);
        moviendoY = true;
    } else if (teclasPresionadas.contains(Qt::Key_Down) ||
               teclasPresionadas.contains(Qt::Key_S)) {
        jugador->moverY(1.0f);
        moviendoY = true;
    }

    if (!moviendoX && !moviendoY) {
        jugador->detener();
    }

    if (teclasPresionadas.contains(Qt::Key_Space)) {
        jugador->iniciarCarga();
    }
}

// ── keyPressEvent ────────────────────────────────────────────────
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (enMenu) {
        if (event->key() == Qt::Key_1) {
            config = DifficultyConfig::facil();
            limpiarMenusUI();
            enMenu = false;
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(nivelInicioActual, escena);
            cambiarMusica(nivelInicioActual);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        } else if (event->key() == Qt::Key_2) {
            config = DifficultyConfig::normal();
            limpiarMenusUI();
            enMenu = false;
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(nivelInicioActual, escena);
            cambiarMusica(nivelInicioActual);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        } else if (event->key() == Qt::Key_3) {
            config = DifficultyConfig::dificil();
            limpiarMenusUI();
            enMenu = false;
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(nivelInicioActual, escena);
            cambiarMusica(nivelInicioActual);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        return;
    }

    teclasPresionadas.insert(event->key());

    if (event->key() == Qt::Key_M) {
        Jugador* jugador = engine->getJugador();
        if (jugador && jugador->isActivo()) {
            jugador->atacar();
            engine->aplicarAtaqueJugador();
            jugador->resetDanio();

            efectoGolpe->play();

            mostrandoGolpe = true;
            contadorGolpe  = 0;

            if (mirandoDerecha) {
                spriteJugador->setPixmap(pixJugadorGolpe);
            } else {
                spriteJugador->setPixmap(pixJugadorGolpeIzq);
            }
        }
    }

    if (event->key() == Qt::Key_P) {
        if (!enMenu) alternarPausa();
        if (timerLoop->isActive()) {
            engine->pausar();
            musicaFondo->pause();
        } else {
            engine->reanudar();
            musicaFondo->play();
        }
    }

    if (event->key() == Qt::Key_R) {
        GameState::EstadoPartida est = engine->getEstado().getEstado();
        if (est == GameState::EstadoPartida::VICTORIA ||
            est == GameState::EstadoPartida::DERROTA) {
            reiniciarJuego();
        }
    }
}

// ── keyReleaseEvent ──────────────────────────────────────────────
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    teclasPresionadas.remove(event->key());

    if (event->key() == Qt::Key_Space) {
        Jugador* jugador = engine->getJugador();
        if (jugador && jugador->isActivo()) {
            jugador->soltarCarga();
        }
    }
}

// ── mousePressEvent ──────────────────────────────────────────────
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPointF posEscena = vista->mapToScene(event->pos());
    QGraphicsItem* itemClickeado = escena->itemAt(posEscena, vista->transform());

    // 1. Detección del bloque de pausa superior izquierdo
    if (itemClickeado == btnPausaFondo || itemClickeado == btnPausaTexto) {
        if (!enMenu) alternarPausa();
        return;
    }

    // 2. Detección de opciones de texto en la pantalla
    QGraphicsTextItem* textoClickeado = dynamic_cast<QGraphicsTextItem*>(itemClickeado);
    if (textoClickeado) {
        QString texto = textoClickeado->toPlainText();

        if (texto == "JUGAR") {
            limpiarMenusUI();
            enMenu = false;
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(nivelInicioActual, escena);
            cambiarFondo(nivelInicioActual);
            cambiarMusica(nivelInicioActual);
            sincronizarSprites();
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        else if (texto.startsWith("SELECCIONAR NIVEL")) {
            nivelInicioActual = (nivelInicioActual == 1) ? 2 : 1;
            dibujarMenuPrincipal(); // Redibuja para actualizar el texto
        }
        else if (texto == "SELECCIONAR DIFICULTAD") {
            dibujarMenuDificultad(juegoPausado);
        }
        else if (texto == "REANUDAR") {
            alternarPausa();
        }
        else if (texto.startsWith("IR AL NIVEL")) {
            int nuevoNivel = (engine->getEstado().getNivel() == 1) ? 2 : 1;
            alternarPausa(); // Esto reanuda los timers internos

            // Destruimos la partida actual y forzamos el inicio en el nuevo nivel
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(nuevoNivel, escena);
            cambiarFondo(nuevoNivel);
            cambiarMusica(nuevoNivel);
            sincronizarSprites();
            setFocus();
        }
        else if (texto == "REGRESAR") {
            if (enSubmenuDesdePausa) dibujarMenuPausa();
            else dibujarMenuPrincipal();
        }
        // ── Ajustes de Dificultad ──
        else if (texto == "FACIL") {
            config = DifficultyConfig::facil();
            if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
        }
        else if (texto == "NORMAL") {
            config = DifficultyConfig::normal();
            if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
        }
        else if (texto == "DIFICIL") {
            config = DifficultyConfig::dificil();
            if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
        }
    } else {
        QMainWindow::mousePressEvent(event);
    }
}

// ── sincronizarSprites ───────────────────────────────────────────
void MainWindow::sincronizarSprites()
{
    for (QGraphicsRectItem* barra : barrasVidaEstaticas) {
        escena->removeItem(barra);
        delete barra;
    }
    barrasVidaEstaticas.clear();

    // ── Efecto de golpe jugador ───────────────────────────────aaaa
    if (mostrandoGolpe) {
        contadorGolpe++;
        if (contadorGolpe >= 12) {
            mostrandoGolpe = false;
            contadorGolpe  = 0;
            if (mirandoDerecha) {
                spriteJugador->setPixmap(pixJugadorNormal);
            } else {
                spriteJugador->setPixmap(pixJugadorNormalIzq);
            }
        }
    }

    // ── Jugador ───────────────────────────────────────────────
    Jugador* jug = engine->getJugador();
    if (jug && spriteJugador) {
        spriteJugador->setPos(jug->getX(), jug->getY());
        bool visible = !jug->isInvulnerable() ||
                       (reloj.elapsed() / 100) % 2 == 0;
        spriteJugador->setVisible(visible);
    }

    const QList<Enemigo*>& enemigos = engine->getEnemigos();

    if (engine->getEstado().getNivel() == 1) {
        for (QGraphicsPixmapItem* s : spritesVariantes) {
            escena->removeItem(s);
            delete s;
        }
        spritesVariantes.clear();
        // ── Levy Nivel 1 ──────────────────────────────────────
        if (!enemigos.isEmpty() && spriteLevy) {
            Enemigo* levy = enemigos[0];
            spriteLevy->setPos(levy->getX(), levy->getY());
            spriteLevy->setVisible(levy->isActivo());

            if (jug && levy->isActivo()) {
                float dx   = levy->getX() - jug->getX();
                float dy   = levy->getY() - jug->getY();
                float dist = std::sqrt(dx * dx + dy * dy);

                bool levyMiraDerecha = jug->getX() > levy->getX();

                if (dist < 80.0f && !levyGolpeando) {
                    levyGolpeando     = true;
                    contadorLevyGolpe = 0;
                    if (levyMiraDerecha) {
                        spriteLevy->setPixmap(pixLevyGolpe);
                    } else {
                        spriteLevy->setPixmap(pixLevyGolpeIzq);
                    }
                } else if (!levyGolpeando) {
                    if (levyMiraDerecha) {
                        spriteLevy->setPixmap(pixLevyNormal);
                    } else {
                        spriteLevy->setPixmap(pixLevyNormalIzq);
                    }
                }

                // Contador para volver al sprite normal
                if (levyGolpeando) {
                    contadorLevyGolpe++;
                    if (contadorLevyGolpe >= 12) {
                        levyGolpeando     = false;
                        contadorLevyGolpe = 0;
                        if (levyMiraDerecha) {
                            spriteLevy->setPixmap(pixLevyNormal);
                        } else {
                            spriteLevy->setPixmap(pixLevyNormalIzq);
                        }
                    }
                }
            }
        }
    } else {
        // Variantes Nivel 2
        if (spriteLevy) spriteLevy->setVisible(false);

        for (QGraphicsPixmapItem* s : spritesVariantes) {
            escena->removeItem(s);
            delete s;
        }
        spritesVariantes.clear();

        QPixmap pixVariante(":/sprites/Sprites/Viltrumincible_85x85.png");
        pixVariante = pixVariante.scaled(60, 80,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

        QPixmap pixVariantePortal(":/sprites/Sprites/Capevincible_85x85.png");
        pixVariantePortal = pixVariantePortal.scaled(50, 50,
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);

        int indexHUD = 0;

        for (Enemigo* e : enemigos) {
            if (!e->isActivo() || !e->isVisible()) continue;;

            // 1. Dibujar el Sprite del enemigo
            QGraphicsPixmapItem* sprVariante = new QGraphicsPixmapItem();

            VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
            if (vp) {
                sprVariante->setPixmap(pixVariantePortal);
            } else {
                sprVariante->setPixmap(pixVariante);
            }

            sprVariante->setPos(e->getX(), e->getY());
            sprVariante->setZValue(2);
            escena->addItem(sprVariante);
            spritesVariantes.append(sprVariante);

            int columna = indexHUD % 3;
            int fila = indexHUD / 3;

            float inicioX = 570.0f;
            float inicioY = 515.0f;
            float anchoBarra = 60.0f;
            float altoBarra = 18.0f;
            float margenX = 15.0f;
            float margenY = 15.0f;

            float xAbsoluto = inicioX + columna * (anchoBarra + margenX);
            float yAbsoluto = inicioY + fila * (altoBarra + margenY);

            float pctVida = e->getVida() / e->getVidaMaxima();
            if (pctVida < 0.0f) pctVida = 0.0f;

            // Fondo de la barra (Rojo oscuro)
            QGraphicsRectItem* fondoBarra = new QGraphicsRectItem(xAbsoluto, yAbsoluto, anchoBarra, altoBarra);
            fondoBarra->setBrush(Qt::darkRed);
            fondoBarra->setPen(Qt::NoPen);
            fondoBarra->setZValue(10); // Siempre por encima del fondo y personajes
            escena->addItem(fondoBarra);
            barrasVidaEstaticas.append(fondoBarra);

            // Nivel de vida (Amarillo)
            QGraphicsRectItem* barraVida = new QGraphicsRectItem(xAbsoluto, yAbsoluto, anchoBarra * pctVida, altoBarra);
            barraVida->setBrush(Qt::yellow);
            barraVida->setPen(Qt::NoPen);
            barraVida->setZValue(11); // Por encima del fondo rojo
            escena->addItem(barraVida);
            barrasVidaEstaticas.append(barraVida);

            indexHUD++; // Avanzar al siguiente hueco en la cuadrícula
        }
    }

    // ── Portal ────────────────────────────────────────────────
    const QList<Portal*>& portales = engine->getPortales();
    if (!portales.isEmpty() && spritePortal && engine->getEstado().getNivel() == 1) {
        Portal* portal = portales[0];
        spritePortal->setPos(portal->getX(), portal->getY());
        spritePortal->setVisible(portal->isActivo());
        float radio = portal->getRadioActual();
        if (radio < 20.0f) radio = 20.0f;
        float escala = radio / 40.0f;
        spritePortal->setScale(escala);
        spritePortal->setRotation(spritePortal->rotation() + 1.5f);
        spritePortal->setOffset(-radio, -radio);
    } else if (engine->getEstado().getNivel() == 2 && spritePortal) {
        spritePortal->setVisible(false);   // Los portales del Nivel 2 se dibujan solos
    }
}

// ── actualizarHUD ────────────────────────────────────────────────
void MainWindow::actualizarHUD()
{
    const GameState& estado = engine->getEstado();

    float pctJug = estado.getVidaJugador() / estado.getVidaMaxJugador();
    if (pctJug < 0.0f) pctJug = 0.0f;
    barraVidaJugador->setRect(10, 515, 150 * pctJug, 18);

    textoPuntos->setPlainText(
        "Puntos: " + QString::number(estado.getPuntos()));

    if (estado.getNivel() == 1) {
        barraVidaLevy->setVisible(true);
        fondoVidaLevy->setVisible(true);
        labelLevy->setVisible(true);

        float pctLevy = estado.getVidaLevy() / estado.getVidaMaxLevy();
        if (pctLevy < 0.0f) pctLevy = 0.0f;
        barraVidaLevy->setRect(620, 515, 150 * pctLevy, 18);

        textoTiempo->setPlainText("");

    } else if (estado.getNivel() == 2) {
        barraVidaLevy->setVisible(false);
        fondoVidaLevy->setVisible(false);
        labelLevy->setVisible(false);

        textoTiempo->setPlainText(
            "Tiempo: " + QString::number(
                (int)estado.getTiempoRestante()) + "s");
    }
}

// ── mostrarPantallaFin ───────────────────────────────────────────
void MainWindow::mostrarPantallaFin(bool victoria)
{
    timerLoop->stop();
    musicaFondo->stop();

    if (victoria && engine->getEstado().getNivel() == 1) {
        QTimer::singleShot(2000, this, [this]() {
            if (textoFinJuego) {
                escena->removeItem(textoFinJuego);
                delete textoFinJuego;
                textoFinJuego = nullptr;
            }
            engine->iniciarNivel(2, escena);
            cambiarFondo(2);
            cambiarMusica(2);
            sincronizarSprites();
            reloj.restart();
            timerLoop->start(16);
        });

        textoFinJuego = new QGraphicsTextItem("NIVEL 2!");
        textoFinJuego->setDefaultTextColor(QColor(0, 255, 100));
        textoFinJuego->setFont(QFont("Arial", 48, QFont::Bold));
        textoFinJuego->setPos(250, 220);
        escena->addItem(textoFinJuego);
        return;
    }

    if (victoria) {
        efectoVictoria->play();
    } else {
        efectoDerrota->play();
    }

    textoFinJuego = new QGraphicsTextItem();
    if (victoria) {
        textoFinJuego->setPlainText("VICTORIA!");
        textoFinJuego->setDefaultTextColor(QColor(0, 255, 100));
    } else {
        textoFinJuego->setPlainText("DERROTA");
        textoFinJuego->setDefaultTextColor(QColor(255, 50, 50));
    }
    textoFinJuego->setFont(QFont("Arial", 48, QFont::Bold));
    textoFinJuego->setPos(180, 180);
    escena->addItem(textoFinJuego);

    textoPuntajeFinal = new QGraphicsTextItem(
        "Puntos: " + QString::number(engine->getEstado().getPuntos()));
    textoPuntajeFinal->setDefaultTextColor(Qt::yellow);
    textoPuntajeFinal->setFont(QFont("Arial", 20));
    textoPuntajeFinal->setPos(300, 280);
    escena->addItem(textoPuntajeFinal);

    textoReiniciar = new QGraphicsTextItem("Presiona R para reiniciar");
    textoReiniciar->setDefaultTextColor(Qt::white);
    textoReiniciar->setFont(QFont("Arial", 16));
    textoReiniciar->setPos(230, 340);
    escena->addItem(textoReiniciar);
}

// ── reiniciarJuego ───────────────────────────────────────────────
void MainWindow::reiniciarJuego()
{
    if (textoFinJuego) {
        escena->removeItem(textoFinJuego);
        delete textoFinJuego;
        textoFinJuego = nullptr;
    }

    if (textoReiniciar) {
        escena->removeItem(textoReiniciar);
        delete textoReiniciar;
        textoReiniciar = nullptr;
    }

    if (textoPuntajeFinal) {
        escena->removeItem(textoPuntajeFinal);
        delete textoPuntajeFinal;
        textoPuntajeFinal = nullptr;
    }

    for (QGraphicsPixmapItem* s : spritesVariantes) {
        escena->removeItem(s);
        delete s;
    }
    spritesVariantes.clear();

    mirandoDerecha    = true;
    mostrandoGolpe    = false;
    contadorGolpe     = 0;
    levyGolpeando     = false;
    contadorLevyGolpe = 0;

    cambiarFondo(1);
    juegoPausado = false;
    dibujarMenuPrincipal();

    cambiarMusica(0);
    setFocus();
}

// Interfaz
void MainWindow::crearBotonPausa()
{
    btnPausaFondo = new QGraphicsRectItem(10, 10, 80, 30);
    btnPausaFondo->setBrush(QBrush(QColor(50, 50, 50, 200)));
    btnPausaFondo->setZValue(100);
    escena->addItem(btnPausaFondo);

    btnPausaTexto = new QGraphicsTextItem("PAUSA");
    btnPausaTexto->setDefaultTextColor(Qt::white);
    btnPausaTexto->setFont(QFont("Arial", 10, QFont::Bold));
    btnPausaTexto->setPos(20, 15);
    btnPausaTexto->setZValue(101);
    escena->addItem(btnPausaTexto);
}

void MainWindow::limpiarMenusUI()
{
    for (QGraphicsItem* item : itemsMenu) {
        escena->removeItem(item);
        delete item;
    }
    itemsMenu.clear();
}

void MainWindow::alternarPausa()
{
    if (!juegoPausado) {
        engine->pausar();
        musicaFondo->pause();
        timerLoop->stop();
        juegoPausado = true;
        dibujarMenuPausa();
    } else {
        limpiarMenusUI();
        engine->reanudar();
        musicaFondo->play();
        reloj.restart();
        timerLoop->start(16);
        juegoPausado = false;
    }
}

// Vistas Dinámicas (Menús)
void MainWindow::dibujarMenuPrincipal()
{
    limpiarMenusUI();
    enMenu = true;

    QGraphicsRectItem* fondo = new QGraphicsRectItem(0, 0, 800, 600);
    fondo->setBrush(QBrush(QColor(0, 0, 0, 190)));
    fondo->setZValue(10);
    escena->addItem(fondo);
    itemsMenu.append(fondo);

    QGraphicsTextItem* titulo = new QGraphicsTextItem("INVINCIBLE KICKBOXING");
    titulo->setDefaultTextColor(QColor(255, 200, 0));
    titulo->setFont(QFont("Arial", 32, QFont::Bold));
    titulo->setPos(80, 120);
    titulo->setZValue(11);
    escena->addItem(titulo);
    itemsMenu.append(titulo);

    QString opciones[] = {"JUGAR", "SELECCIONAR NIVEL (" + QString::number(nivelInicioActual) + ")", "SELECCIONAR DIFICULTAD"};
    for (int i = 0; i < 3; i++) {
        QGraphicsTextItem* texto = new QGraphicsTextItem(opciones[i]);
        texto->setDefaultTextColor(Qt::white);
        texto->setFont(QFont("Arial", 18, QFont::Bold));
        texto->setPos(250, 260 + (i * 60));
        texto->setZValue(11);
        escena->addItem(texto);
        itemsMenu.append(texto);
    }
}

void MainWindow::dibujarMenuPausa()
{
    limpiarMenusUI();

    QGraphicsRectItem* fondo = new QGraphicsRectItem(0, 0, 800, 600);
    fondo->setBrush(QBrush(QColor(0, 0, 0, 190)));
    fondo->setZValue(10);
    escena->addItem(fondo);
    itemsMenu.append(fondo);

    QGraphicsTextItem* titulo = new QGraphicsTextItem("PAUSADO");
    titulo->setDefaultTextColor(Qt::white);
    titulo->setFont(QFont("Arial", 32, QFont::Bold));
    titulo->setPos(280, 120);
    titulo->setZValue(11);
    escena->addItem(titulo);
    itemsMenu.append(titulo);

    int nivelActual = engine->getEstado().getNivel();
    int proximoNivel = (nivelActual == 1) ? 2 : 1;

    QString opciones[] = {"REANUDAR", "IR AL NIVEL " + QString::number(proximoNivel), "SELECCIONAR DIFICULTAD"};
    for (int i = 0; i < 3; i++) {
        QGraphicsTextItem* texto = new QGraphicsTextItem(opciones[i]);
        texto->setDefaultTextColor(Qt::white);
        texto->setFont(QFont("Arial", 18, QFont::Bold));
        texto->setPos(260, 260 + (i * 60));
        texto->setZValue(11);
        escena->addItem(texto);
        itemsMenu.append(texto);
    }
}

void MainWindow::dibujarMenuDificultad(bool desdePausa)
{
    limpiarMenusUI();
    enSubmenuDesdePausa = desdePausa;

    QGraphicsRectItem* fondo = new QGraphicsRectItem(0, 0, 800, 600);
    fondo->setBrush(QBrush(QColor(0, 0, 0, 190)));
    fondo->setZValue(10);
    escena->addItem(fondo);
    itemsMenu.append(fondo);

    QString opciones[] = {"REGRESAR", "FACIL", "NORMAL", "DIFICIL"};
    QColor colores[] = {Qt::white, QColor(0, 220, 100), QColor(255, 200, 0), QColor(220, 50, 50)};

    for (int i = 0; i < 4; i++) {
        QGraphicsTextItem* texto = new QGraphicsTextItem(opciones[i]);
        texto->setDefaultTextColor(colores[i]);
        texto->setFont(QFont("Arial", 20, QFont::Bold));
        texto->setPos(320, 200 + (i * 60));
        texto->setZValue(11);
        escena->addItem(texto);
        itemsMenu.append(texto);
    }
}
