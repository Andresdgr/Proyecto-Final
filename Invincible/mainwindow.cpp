#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QTransform>
#include <QCoreApplication>
#include <cmath>
#include "varianteportal.h"

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
    , fondoMenu(nullptr)
    , textoTitulo(nullptr)
    , textoSubtitulo(nullptr)
    , textoFacil(nullptr)
    , textoNormal(nullptr)
    , textoDificil(nullptr)
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

    mostrarMenu();
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
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            cambiarMusica(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        } else if (event->key() == Qt::Key_2) {
            config = DifficultyConfig::normal();
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            cambiarMusica(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        } else if (event->key() == Qt::Key_3) {
            config = DifficultyConfig::dificil();
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            cambiarMusica(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        return;
    }

    teclasPresionadas.insert(event->key());

    if (event->key() == Qt::Key_Z) {
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

    if (event->key() == Qt::Key_Escape) {
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

// ── sincronizarSprites ───────────────────────────────────────────
void MainWindow::sincronizarSprites()
{
    // ── Efecto de golpe jugador ───────────────────────────────
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
        // ── Clones y proyectiles Nivel 2 ──────────────────────
        if (spriteLevy) spriteLevy->setVisible(false);

        for (QGraphicsPixmapItem* s : spritesClones) {
            escena->removeItem(s);
            delete s;
        }
        spritesClones.clear();

        QPixmap pixVariante(":/sprites/Sprites/Capevincible_85x85.png");
        pixVariante = pixVariante.scaled(60, 80,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

        QPixmap pixProyectil(":/sprites/Sprites/Viltrumincible_85x85.png");
        pixProyectil = pixProyectil.scaled(50, 50,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

        for (Enemigo* e : enemigos) {
            if (!e->isActivo()) continue;

            QGraphicsPixmapItem* sprClon = new QGraphicsPixmapItem();

            VariantePortal* vp = dynamic_cast<VariantePortal*>(e);
            if (vp) {
                sprClon->setPixmap(pixProyectil);
            } else {
                sprClon->setPixmap(pixVariante);
            }

            sprClon->setPos(e->getX(), e->getY());
            sprClon->setZValue(2);
            escena->addItem(sprClon);
            spritesClones.append(sprClon);
        }
    }

    // ── Portal ────────────────────────────────────────────────
    const QList<Portal*>& portales = engine->getPortales();
    if (!portales.isEmpty() && spritePortal) {
        Portal* portal = portales[0];
        spritePortal->setPos(portal->getX(), portal->getY());
        spritePortal->setVisible(portal->isActivo());
        float radio = portal->getRadioActual();
        if (radio < 20.0f) radio = 20.0f;
        float escala = radio / 40.0f;
        spritePortal->setScale(escala);
        spritePortal->setRotation(spritePortal->rotation() + 1.5f);
        spritePortal->setOffset(-radio, -radio);
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
            engine->iniciarNivel(2);
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

    for (QGraphicsPixmapItem* s : spritesClones) {
        escena->removeItem(s);
        delete s;
    }
    spritesClones.clear();

    mirandoDerecha    = true;
    mostrandoGolpe    = false;
    contadorGolpe     = 0;
    levyGolpeando     = false;
    contadorLevyGolpe = 0;

    cambiarFondo(1);
    mostrarMenu();
    cambiarMusica(0);
    setFocus();
}

// ── mostrarMenu ──────────────────────────────────────────────────
void MainWindow::mostrarMenu()
{
    enMenu = true;
    timerLoop->stop();

    fondoMenu = new QGraphicsRectItem(0, 0, 800, 600);
    fondoMenu->setBrush(QBrush(QColor(0, 0, 0, 180)));
    fondoMenu->setPen(Qt::NoPen);
    fondoMenu->setZValue(10);
    escena->addItem(fondoMenu);

    textoTitulo = new QGraphicsTextItem("INVINCIBLE KICKBOXER");
    textoTitulo->setDefaultTextColor(QColor(255, 200, 0));
    textoTitulo->setFont(QFont("Arial", 32, QFont::Bold));
    textoTitulo->setPos(80, 120);
    textoTitulo->setZValue(11);
    escena->addItem(textoTitulo);

    textoSubtitulo = new QGraphicsTextItem("Selecciona la dificultad:");
    textoSubtitulo->setDefaultTextColor(Qt::white);
    textoSubtitulo->setFont(QFont("Arial", 16));
    textoSubtitulo->setPos(240, 220);
    textoSubtitulo->setZValue(11);
    escena->addItem(textoSubtitulo);

    textoFacil = new QGraphicsTextItem("[ 1 ]  FACIL");
    textoFacil->setDefaultTextColor(QColor(0, 220, 100));
    textoFacil->setFont(QFont("Arial", 20, QFont::Bold));
    textoFacil->setPos(280, 300);
    textoFacil->setZValue(11);
    escena->addItem(textoFacil);

    textoNormal = new QGraphicsTextItem("[ 2 ]  NORMAL");
    textoNormal->setDefaultTextColor(QColor(255, 200, 0));
    textoNormal->setFont(QFont("Arial", 20, QFont::Bold));
    textoNormal->setPos(280, 360);
    textoNormal->setZValue(11);
    escena->addItem(textoNormal);

    textoDificil = new QGraphicsTextItem("[ 3 ]  DIFICIL");
    textoDificil->setDefaultTextColor(QColor(220, 50, 50));
    textoDificil->setFont(QFont("Arial", 20, QFont::Bold));
    textoDificil->setPos(280, 420);
    textoDificil->setZValue(11);
    escena->addItem(textoDificil);
}

// ── ocultarMenu ──────────────────────────────────────────────────
void MainWindow::ocultarMenu()
{
    if (fondoMenu)     { escena->removeItem(fondoMenu);     delete fondoMenu;     fondoMenu     = nullptr; }
    if (textoTitulo)   { escena->removeItem(textoTitulo);   delete textoTitulo;   textoTitulo   = nullptr; }
    if (textoSubtitulo){ escena->removeItem(textoSubtitulo);delete textoSubtitulo;textoSubtitulo= nullptr; }
    if (textoFacil)    { escena->removeItem(textoFacil);    delete textoFacil;    textoFacil    = nullptr; }
    if (textoNormal)   { escena->removeItem(textoNormal);   delete textoNormal;   textoNormal   = nullptr; }
    if (textoDificil)  { escena->removeItem(textoDificil);  delete textoDificil;  textoDificil  = nullptr; }

    enMenu = false;
}
