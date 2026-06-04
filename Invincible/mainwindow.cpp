#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QFont>
#include <QPainter>

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
    , barraVidaJugador(nullptr)
    , barraVidaLevy(nullptr)
    , textoPuntos(nullptr)
    , textoTiempo(nullptr)
    , textoFinJuego(nullptr)
    , textoReiniciar(nullptr)
    , textoPuntajeFinal(nullptr)
    , textoTitulo(nullptr)
    , textoFacil(nullptr)
    , textoNormal(nullptr)
    , textoDificil(nullptr)
    , enMenu(true)
    , fondoMenu(nullptr)
    , textoSubtitulo(nullptr)

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

    // Mostrar menú al inicio en lugar de iniciar el juego directamente
    mostrarMenu();
    setFocus();
}

// ── Destructor ───────────────────────────────────────────────────
MainWindow::~MainWindow()
{
    delete engine;
    delete ui;
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

    // ── Fondo del escenario — detrás de todo ─────────────────
    QPixmap pixFondo(":/sprites/assets/sprites/Escenario.png");
    pixFondo = pixFondo.scaled(800, 600,
                               Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
    QGraphicsPixmapItem* itemFondo = new QGraphicsPixmapItem(pixFondo);
    itemFondo->setPos(0, 0);
    itemFondo->setZValue(-1); // siempre detrás
    escena->addItem(itemFondo);

    // ── Sprite jugador ────────────────────────────────────────
    spriteJugador = new QGraphicsPixmapItem();
    QPixmap pixJugador(":/sprites/assets/sprites/Invincible.png");
    pixJugador = pixJugador.scaled(60, 80,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    spriteJugador->setPixmap(pixJugador);
    spriteJugador->setPos(400, 300);
    spriteJugador->setZValue(1);
    escena->addItem(spriteJugador);

    // ── Sprite Levy ───────────────────────────────────────────
    spriteLevy = new QGraphicsPixmapItem();
    QPixmap pixLevy(":/sprites/assets/sprites/Amstrongr.png");
    pixLevy = pixLevy.scaled(70, 90,
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    spriteLevy->setPixmap(pixLevy);
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
void MainWindow::mostrarPantallaFin(bool victoria)
{
    timerLoop->stop();

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

    // Usar atributo en lugar de variable local
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

    // Volver al menú en lugar de reiniciar directamente
    mostrarMenu();
    setFocus();
}
// ── inicializarHUD ───────────────────────────────────────────────
void MainWindow::inicializarHUD()
{
    // ── Barra vida jugador ────────────────────────────────────
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

    // ── Barra vida Levy ───────────────────────────────────────
    QGraphicsRectItem* fondoVidaL = new QGraphicsRectItem(620, 515, 150, 18);
    fondoVidaL->setBrush(QBrush(QColor(80, 80, 80)));
    fondoVidaL->setPen(Qt::NoPen);
    escena->addItem(fondoVidaL);

    barraVidaLevy = new QGraphicsRectItem(620, 515, 150, 18);
    barraVidaLevy->setBrush(QBrush(QColor(200, 50, 50)));
    barraVidaLevy->setPen(Qt::NoPen);
    escena->addItem(barraVidaLevy);

    QGraphicsTextItem* labelL = new QGraphicsTextItem("Angstrom Levy");
    labelL->setDefaultTextColor(Qt::white);
    labelL->setFont(QFont("Arial", 8));
    labelL->setPos(620, 535);
    escena->addItem(labelL);

    // ── Puntos ────────────────────────────────────────────────
    textoPuntos = new QGraphicsTextItem("Puntos: 0");
    textoPuntos->setDefaultTextColor(Qt::yellow);
    textoPuntos->setFont(QFont("Arial", 11, QFont::Bold));
    textoPuntos->setPos(340, 512);
    escena->addItem(textoPuntos);

    // ── Tiempo ────────────────────────────────────────────────
    textoTiempo = new QGraphicsTextItem("");
    textoTiempo->setDefaultTextColor(Qt::cyan);
    textoTiempo->setFont(QFont("Arial", 11, QFont::Bold));
    textoTiempo->setPos(340, 530);
    escena->addItem(textoTiempo);
}

// ── onUpdate — slot del game loop ────────────────────────────────
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

    // Verificar condición de fin
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

    // Movimiento horizontal
    if (teclasPresionadas.contains(Qt::Key_Left) ||
        teclasPresionadas.contains(Qt::Key_A)) {
        jugador->moverX(-1.0f);
        moviendoX = true;
    }
    else if (teclasPresionadas.contains(Qt::Key_Right) ||
             teclasPresionadas.contains(Qt::Key_D)) {
        jugador->moverX(1.0f);
        moviendoX = true;
    }

    // Movimiento vertical
    if (teclasPresionadas.contains(Qt::Key_Up) ||
        teclasPresionadas.contains(Qt::Key_W)) {
        jugador->moverY(-1.0f);
        moviendoY = true;
    }
    else if (teclasPresionadas.contains(Qt::Key_Down) ||
             teclasPresionadas.contains(Qt::Key_S)) {
        jugador->moverY(1.0f);
        moviendoY = true;
    }

    // Solo detener si no hay ninguna tecla de movimiento presionada
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
    // ── Controles del menú ────────────────────────────────────
    if (enMenu) {
        if (event->key() == Qt::Key_1) {
            config = DifficultyConfig::facil();
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        else if (event->key() == Qt::Key_2) {
            config = DifficultyConfig::normal();
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        else if (event->key() == Qt::Key_3) {
            config = DifficultyConfig::dificil();
            ocultarMenu();
            delete engine;
            engine = new GameEngine(config);
            engine->iniciarNivel(1);
            reloj.restart();
            timerLoop->start(16);
            setFocus();
        }
        return; // no procesar otras teclas mientras está el menú
    }

    // ── Controles del juego ───────────────────────────────────
    teclasPresionadas.insert(event->key());

    if (event->key() == Qt::Key_Z) {
        Jugador* jugador = engine->getJugador();
        if (jugador && jugador->isActivo()) {
            jugador->atacar();
            engine->aplicarAtaqueJugador();
            jugador->resetDanio();
        }
    }

    if (event->key() == Qt::Key_Escape) {
        if (timerLoop->isActive())
            engine->pausar();
        else
            engine->reanudar();
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

    // No propagar
}

// ── sincronizarSprites ───────────────────────────────────────────
void MainWindow::sincronizarSprites()
{
    // ── Jugador ───────────────────────────────────────────────
    Jugador* jug = engine->getJugador();
    if (jug && spriteJugador) {
        spriteJugador->setPos(jug->getX(), jug->getY());
        bool visible = !jug->isInvulnerable() ||
                       (reloj.elapsed() / 100) % 2 == 0;
        spriteJugador->setVisible(visible);
    }

    // ── Levy ──────────────────────────────────────────────────
    const QList<Enemigo*>& enemigos = engine->getEnemigos();
    if (!enemigos.isEmpty() && spriteLevy) {
        Enemigo* levy = enemigos[0];
        spriteLevy->setPos(levy->getX(), levy->getY());
        spriteLevy->setVisible(levy->isActivo());
    }

    // ── Portal ────────────────────────────────────────────────
    const QList<Portal*>& portales = engine->getPortales();
    if (!portales.isEmpty() && spritePortal) {
        Portal* portal = portales[0];
        spritePortal->setPos(portal->getX(), portal->getY());
        spritePortal->setVisible(portal->isActivo());

        // Escalar según el radio del MAS
        float radio = portal->getRadioActual();
        if (radio < 20.0f) radio = 20.0f;

        // Escala proporcional al radio — base 80px → escala 1.0
        float escala = radio / 40.0f;
        spritePortal->setScale(escala);

        // Rotar el portal para efecto visual
        spritePortal->setRotation(spritePortal->rotation() + 1.5f);

        // Centrar el sprite en su posición
        spritePortal->setOffset(-radio, -radio);
    }
}

// ── actualizarHUD ────────────────────────────────────────────────
void MainWindow::actualizarHUD()
{
    const GameState& estado = engine->getEstado();

    // Barra vida jugador
    float pctJug = estado.getVidaJugador() / estado.getVidaMaxJugador();
    if (pctJug < 0.0f) pctJug = 0.0f;
    barraVidaJugador->setRect(10, 515, 150 * pctJug, 18);

    // Barra vida Levy
    float pctLevy = estado.getVidaLevy() / estado.getVidaMaxLevy();
    if (pctLevy < 0.0f) pctLevy = 0.0f;
    barraVidaLevy->setRect(620, 515, 150 * pctLevy, 18);

    // Puntos
    textoPuntos->setPlainText(
        "Puntos: " + QString::number(estado.getPuntos()));

    // Tiempo solo en Nivel 2
    if (estado.getNivel() == 2) {
        textoTiempo->setPlainText(
            "Tiempo: " + QString::number(
                (int)estado.getTiempoRestante()) + "s");
    }
}
void MainWindow::mostrarMenu()
{
    enMenu = true;
    timerLoop->stop();

    // Fondo semitransparente oscuro
    QGraphicsRectItem* fondo = new QGraphicsRectItem(0, 0, 800, 600);
    fondo->setBrush(QBrush(QColor(0, 0, 0, 180)));
    fondo->setPen(Qt::NoPen);
    fondo->setZValue(10);
    escena->addItem(fondoMenu);

    // Título
    textoTitulo = new QGraphicsTextItem("INVINCIBLE KICKBOXER");
    textoTitulo->setDefaultTextColor(QColor(255, 200, 0));
    textoTitulo->setFont(QFont("Arial", 32, QFont::Bold));
    textoTitulo->setPos(80, 120);
    textoTitulo->setZValue(11);
    escena->addItem(textoTitulo);

    // Subtítulo
    QGraphicsTextItem* sub = new QGraphicsTextItem("Selecciona la dificultad:");
    sub->setDefaultTextColor(Qt::white);
    sub->setFont(QFont("Arial", 16));
    sub->setPos(240, 220);
    sub->setZValue(11);
    escena->addItem(textoSubtitulo);

    // Opción Fácil
    textoFacil = new QGraphicsTextItem("[ 1 ]  FACIL");
    textoFacil->setDefaultTextColor(QColor(0, 220, 100));
    textoFacil->setFont(QFont("Arial", 20, QFont::Bold));
    textoFacil->setPos(280, 300);
    textoFacil->setZValue(11);
    escena->addItem(textoFacil);

    // Opción Normal
    textoNormal = new QGraphicsTextItem("[ 2 ]  NORMAL");
    textoNormal->setDefaultTextColor(QColor(255, 200, 0));
    textoNormal->setFont(QFont("Arial", 20, QFont::Bold));
    textoNormal->setPos(280, 360);
    textoNormal->setZValue(11);
    escena->addItem(textoNormal);

    // Opción Difícil
    textoDificil = new QGraphicsTextItem("[ 3 ]  DIFICIL");
    textoDificil->setDefaultTextColor(QColor(220, 50, 50));
    textoDificil->setFont(QFont("Arial", 20, QFont::Bold));
    textoDificil->setPos(280, 420);
    textoDificil->setZValue(11);
    escena->addItem(textoDificil);

}
void MainWindow::ocultarMenu()
{
    if (fondoMenu)    { escena->removeItem(fondoMenu);    delete fondoMenu;    fondoMenu    = nullptr; }
    // Eliminar todos los items del menú de la escena
    if (textoTitulo)  { escena->removeItem(textoTitulo);  delete textoTitulo;  textoTitulo  = nullptr; }
    if (textoSubtitulo){ escena->removeItem(textoSubtitulo);delete textoSubtitulo;textoSubtitulo= nullptr; }
    if (textoFacil)   { escena->removeItem(textoFacil);   delete textoFacil;   textoFacil   = nullptr; }
    if (textoNormal)  { escena->removeItem(textoNormal);  delete textoNormal;  textoNormal  = nullptr; }
    if (textoDificil) { escena->removeItem(textoDificil); delete textoDificil; textoDificil = nullptr; }

    enMenu = false;
}
