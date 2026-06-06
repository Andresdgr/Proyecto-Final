#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QTransform>
#include <QCoreApplication>
#include <QApplication>
#include <cmath>
#include "jugador.h"
#include "varianteportal.h"
#include "variante.h"
#include "angstromlevy.h"


// ── Inicializar audio ─────────────────────────────────────
static const QString MUSICA_MENU   = "qrc:/soundtrack/win.mp3";
static const QString MUSICA_NIVEL1 = "qrc:/soundtrack/levy.mp3";
static const QString MUSICA_NIVEL2 = "qrc:/soundtrack/Tom_tom.mp3";

static const QString SFX_GOLPE     = "qrc:/soundtrack/golpe.wav";
static const QString SFX_VICTORIA  = "qrc:/soundtrack/win.wav";
static const QString SFX_DERROTA   = "qrc:/soundtrack/lose.wav";
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
    , mostrandoGolpe(false)
    , contadorGolpe(0)
    , mirandoDerecha(true)
    , pixVarianteBase(QPixmap())
    , pixVarianteWalk(QPixmap())
    , pixVarianteWalkIzq(QPixmap())
    , pixVarianteAttack(QPixmap())
    , pixVarianteAttackIzq(QPixmap())
    , pixVariantePortalBase(QPixmap())
    , pixVariantePortalWalk(QPixmap())
    , pixVariantePortalWalkIzq(QPixmap())
    , pixVariantePortalAttack(QPixmap())
    , pixVariantePortalAttackIzq(QPixmap())
    , contadorAnimacion(0)
    , jugadorMoviendose(false)
    , levyGolpeando(false)
    , contadorLevyGolpe(0)
    , barraVidaJugador(nullptr)
    , barraVidaLevy(nullptr)
    , fondoVidaLevy(nullptr)
    , labelLevy(nullptr)
    , textoPuntos(nullptr)
    , textoTiempo(nullptr)
    , textoFinJuego(nullptr)
    , textoReiniciar(nullptr)
    , textoPuntajeFinal(nullptr)
    , textoSalir(nullptr)
<<<<<<< HEAD
    , fondoMenu(nullptr)
    , textoTitulo(nullptr)
    , textoSubtitulo(nullptr)
    , textoFacil(nullptr)
    , textoNormal(nullptr)
    , textoDificil(nullptr)
=======
    , btnPausaFondo(nullptr)
    , btnPausaTexto(nullptr)
>>>>>>> origin/main
    , enMenu(true)
    , juegoPausado(false)
    , enSubmenuDesdePausa(false)
    , nivelInicioActual(1)
    , musicaFondo(nullptr)
    , audioFondo(nullptr)
    , efectoGolpe(nullptr)
    , efectoVictoria(nullptr)
    , efectoDerrota(nullptr)
    , contadorAnimacion(0)
    , jugadorMoviendose(false)
{
    ui->setupUi(this);
    setWindowTitle("Invincible Kickboxing");
    setFixedSize(800, 600);

    engine = new GameEngine(config);

    inicializarEscena();
    inicializarHUD();

    timerLoop = new QTimer(this);
    connect(timerLoop, &QTimer::timeout,
            this,      &MainWindow::onUpdate);

    reloj.start();


    // ── Música de fondo
    musicaFondo = new QMediaPlayer(this);
    audioFondo  = new QAudioOutput(this);
    musicaFondo->setAudioOutput(audioFondo);
    audioFondo->setVolume(0.4f);
    musicaFondo->setLoops(QMediaPlayer::Infinite);

    // ── Efectos de sonido
    efectoGolpe = new QSoundEffect(this);
    efectoGolpe->setSource(QUrl(SFX_GOLPE));
    efectoGolpe->setVolume(0.5f);

    efectoVictoria = new QSoundEffect(this);
    efectoVictoria->setSource(QUrl(SFX_VICTORIA));
    efectoVictoria->setVolume(1.0f);

    efectoDerrota = new QSoundEffect(this);
    efectoDerrota->setSource(QUrl(SFX_DERROTA));
    efectoDerrota->setVolume(1.0f);

    dibujarMenuPrincipal(); // Se ajustó a tu método actual
    cambiarMusica(0);
    setFocus();
}

// ── Destructor ───────────────────────────────────────────────────
MainWindow::~MainWindow()
{
    delete engine;
    delete ui;
}

// ── iniciarJuego ─────────────────────────────────────────────────
void MainWindow::iniciarJuego(int nivel)
{
    limpiarMenusUI();
    enMenu = false;
    delete engine;
    engine = new GameEngine(config);
    engine->iniciarNivel(nivel);
    cambiarFondo(nivel);
    cambiarMusica(nivel);
    sincronizarSprites();
    reloj.restart();
    timerLoop->start(16);
    setFocus();
}

// ── cambiarMusica ────────────────────────────────────────────────
void MainWindow::cambiarMusica(int estadoMusica)
{
    efectoVictoria->stop();
    efectoDerrota->stop();

    QUrl nuevaRuta;

    switch (estadoMusica) {
    case 0: nuevaRuta = QUrl(MUSICA_MENU);   break; // Menú
    case 1: nuevaRuta = QUrl(MUSICA_NIVEL1); break; // Nivel 1
    case 2: nuevaRuta = QUrl(MUSICA_NIVEL2); break; // Nivel 2
    case 3: // Victoria Nivel 1
    case 4: // Victoria Nivel 2
        musicaFondo->stop();
        efectoVictoria->play();
        return;
    case 5: // Derrota Nivel 1
    case 6: // Derrota Nivel 2
        musicaFondo->stop();
        efectoDerrota->play();
        return;
    default:
        nuevaRuta = QUrl(MUSICA_MENU);
        break;
    }

    if (musicaFondo->source() != nuevaRuta) {
        musicaFondo->stop();
        musicaFondo->setSource(nuevaRuta);
        musicaFondo->play();
    } else if (musicaFondo->playbackState() != QMediaPlayer::PlayingState) {
        musicaFondo->play();
    }
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
        pixFondo = QPixmap(":/sprites/Sprites/Escenario.png");
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

    pixJugadorNormal = QPixmap(":/sprites/Sprites/Invincible_base.png");
    pixJugadorNormal = pixJugadorNormal.scaled(60, 80,
                                               Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixJugadorNormalIzq = pixJugadorNormal.transformed(
        QTransform().scale(-1, 1));

<<<<<<< HEAD
    pixJugadorGolpe = QPixmap(":/sprites/assets/sprites/invinsibleGolpeSinFondo.png");
    pixJugadorGolpe = pixJugadorGolpe.scaled(60, 80,
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);

    pixJugadorGolpeIzq = pixJugadorGolpe.transformed(
        QTransform().scale(-1, 1));

    pixJugadorWalk = QPixmap(":/sprites/assets/sprites/invincible_walk1.png");
    pixJugadorWalk = pixJugadorWalk.scaled(60, 80,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

=======
    pixJugadorGolpe = QPixmap(":/sprites/Sprites/Invincible_ataque.png");
    pixJugadorGolpe = pixJugadorGolpe.scaled(60, 80,
                                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixJugadorGolpeIzq = pixJugadorGolpe.transformed(
        QTransform().scale(-1, 1));

    pixJugadorWalk = QPixmap(":/sprites/Sprites/Invincible_mov.png");
    pixJugadorWalk = pixJugadorWalk.scaled(60, 80,
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
>>>>>>> origin/main
    pixJugadorWalkIzq = pixJugadorWalk.transformed(
        QTransform().scale(-1, 1));

    spriteJugador->setPixmap(pixJugadorNormal);
    spriteJugador->setPos(400, 300);
    spriteJugador->setZValue(1);
    escena->addItem(spriteJugador);

    // ── Sprite Levy ───────────────────────────────────────────
    spriteLevy = new QGraphicsPixmapItem();

    pixLevyNormal = QPixmap(":/sprites/Sprites/Amstrong.png");
    pixLevyNormal = pixLevyNormal.scaled(70, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixLevyNormalIzq = pixLevyNormal.transformed(QTransform().scale(-1, 1));

    // CARGAR SPRITE DE CAMINATA
    pixLevyWalk = QPixmap(":/sprites/Sprites/Amstrong_mov.png"); // Cambia este nombre al archivo real
    if (pixLevyWalk.isNull()) pixLevyWalk = pixLevyNormal; // Seguridad: usa el normal si no encuentra el archivo
    pixLevyWalk = pixLevyWalk.scaled(70, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixLevyWalkIzq = pixLevyWalk.transformed(QTransform().scale(-1, 1));

    pixLevyGolpe = QPixmap(":/sprites/Sprites/Amstrong_ataque.png");
    pixLevyGolpe = pixLevyGolpe.scaled(70, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixLevyGolpeIzq = pixLevyGolpe.transformed(QTransform().scale(-1, 1));

    spriteLevy->setPixmap(pixLevyNormal);
    spriteLevy->setPos(600, 300);
    spriteLevy->setZValue(1);
    escena->addItem(spriteLevy);

    // ── Sprite Portal (solo Nivel 1) ──────────────────────────
    spritePortal = new QGraphicsPixmapItem();
    QPixmap pixPortal(":/sprites/Sprites/Portal.png");
    pixPortal = pixPortal.scaled(80, 80,
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
    spritePortal->setPixmap(pixPortal);
    spritePortal->setPos(500, 200);
    spritePortal->setZValue(1);
    escena->addItem(spritePortal);

    // 1. Sprite Base (Quieto o movimiento puramente vertical en Y)
    pixVarianteBase = QPixmap(":/sprites/Sprites/Movincihawk_base.png");
    pixVarianteBase = pixVarianteBase.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 2. Sprite de Caminata/Desplazamiento (Modifica la ruta si creas uno específico)
    pixVarianteWalk = QPixmap(":/sprites/Sprites/Movincihawk_mov.png");
    if (pixVarianteWalk.isNull()) pixVarianteWalk = pixVarianteBase; // Seguridad: Usa el base si no existe
    pixVarianteWalk = pixVarianteWalk.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixVarianteWalkIzq = pixVarianteWalk.transformed(QTransform().scale(-1, 1)); // Inversión horizontal automática

    // 3. Sprite de Ataque (Modifica la ruta si tienes uno específico)
    pixVarianteAttack = QPixmap(":/sprites/Sprites/Movincihawk_ataque.png");
    if (pixVarianteAttack.isNull()) pixVarianteAttack = pixVarianteBase;
    pixVarianteAttack = pixVarianteAttack.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixVarianteAttackIzq = pixVarianteAttack.transformed(QTransform().scale(-1, 1)); // Inversión horizontal automática

    // ── Sprites VariantePortal (voladora) ─────────────────────
    pixVariantePortalBase = QPixmap(":/sprites/Sprites/Capevincible_base.png");
    pixVariantePortalBase = pixVariantePortalBase.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    pixVariantePortalWalk = QPixmap(":/sprites/Sprites/Capevincible_mov.png");
    if (pixVariantePortalWalk.isNull()) pixVariantePortalWalk = pixVariantePortalBase;
    pixVariantePortalWalk    = pixVariantePortalWalk.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixVariantePortalWalkIzq = pixVariantePortalWalk.transformed(QTransform().scale(-1, 1));

    pixVariantePortalAttack = QPixmap(":/sprites/Sprites/Capevincible_ataque.png");
    if (pixVariantePortalAttack.isNull()) pixVariantePortalAttack = pixVariantePortalBase;
    pixVariantePortalAttack    = pixVariantePortalAttack.scaled(60, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixVariantePortalAttackIzq = pixVariantePortalAttack.transformed(QTransform().scale(-1, 1));
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
        moviendoX         = true;
        mirandoDerecha    = false;
        jugadorMoviendose = true;
    } else if (teclasPresionadas.contains(Qt::Key_Right) ||
               teclasPresionadas.contains(Qt::Key_D)) {
        jugador->moverX(1.0f);
        moviendoX         = true;
        mirandoDerecha    = true;
        jugadorMoviendose = true;
    }

    if (teclasPresionadas.contains(Qt::Key_Up) ||
        teclasPresionadas.contains(Qt::Key_W)) {
        jugador->moverY(-1.0f);
        moviendoY         = true;
        jugadorMoviendose = true;
    } else if (teclasPresionadas.contains(Qt::Key_Down) ||
               teclasPresionadas.contains(Qt::Key_S)) {
        jugador->moverY(1.0f);
        moviendoY         = true;
        jugadorMoviendose = true;
    }

    if (!moviendoX && !moviendoY) {
        jugador->detener();
        jugadorMoviendose = false;
    }

    if (teclasPresionadas.contains(Qt::Key_Space)) {
        jugador->iniciarCarga();
    }
}

// ── keyPressEvent ────────────────────────────────────────────────
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (enMenu) return;

    teclasPresionadas.insert(event->key());

    if (event->key() == Qt::Key_M) {
        Jugador* jugador = engine->getJugador();
        if (jugador && jugador->isActivo()) {
            jugador->atacar();
            engine->aplicarAtaqueJugador();
            jugador->resetDanio();
            efectoGolpe->play();
<<<<<<< HEAD

            mostrandoGolpe    = true;
            contadorGolpe     = 0;
            jugadorMoviendose = false;

=======
            mostrandoGolpe    = true;
            contadorGolpe     = 0;
            jugadorMoviendose = false;
>>>>>>> origin/main
            if (mirandoDerecha) {
                spriteJugador->setPixmap(pixJugadorGolpe);
            } else {
                spriteJugador->setPixmap(pixJugadorGolpeIzq);
            }
        }
    }

    if (event->key() == Qt::Key_P) {
        alternarPausa();
    }

    if (event->key() == Qt::Key_Escape) {
        GameState::EstadoPartida est = engine->getEstado().getEstado();
        if (est == GameState::EstadoPartida::VICTORIA ||
            est == GameState::EstadoPartida::DERROTA) {
            QApplication::quit();
<<<<<<< HEAD
        } else if (timerLoop->isActive()) {
            engine->pausar();
            musicaFondo->pause();
=======
>>>>>>> origin/main
        } else {
            alternarPausa();
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

    if (itemClickeado == btnPausaFondo || itemClickeado == btnPausaTexto) {
        if (!enMenu) alternarPausa();
        return;
    }

    QGraphicsTextItem* textoClickeado = dynamic_cast<QGraphicsTextItem*>(itemClickeado);
    if (!textoClickeado) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    QString texto = textoClickeado->toPlainText();

    if (texto == "JUGAR") {
        iniciarJuego(nivelInicioActual);
    } else if (texto.startsWith("SELECCIONAR NIVEL")) {
        nivelInicioActual = (nivelInicioActual == 1) ? 2 : 1;
        dibujarMenuPrincipal();
    } else if (texto == "SELECCIONAR DIFICULTAD") {
        dibujarMenuDificultad(juegoPausado);
    } else if (texto == "REANUDAR") {
        alternarPausa();
    } else if (texto.startsWith("IR AL NIVEL")) {
        int nuevoNivel = (engine->getEstado().getNivel() == 1) ? 2 : 1;
        alternarPausa();
        iniciarJuego(nuevoNivel);
    } else if (texto == "CONTINUAR AL NIVEL 2") {
        iniciarJuego(2);
    } else if (texto == "REGRESAR AL MENU") {
        reiniciarJuego();
    } else if (texto == "REGRESAR") {
        if (enSubmenuDesdePausa) dibujarMenuPausa();
        else dibujarMenuPrincipal();
    } else if (texto == "FACIL") {
        config = DifficultyConfig::facil();
        if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
    } else if (texto == "NORMAL") {
        config = DifficultyConfig::normal();
        if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
    } else if (texto == "DIFICIL") {
        config = DifficultyConfig::dificil();
        if (enSubmenuDesdePausa) dibujarMenuPausa(); else dibujarMenuPrincipal();
    }
}

// ── sincronizarSprites ───────────────────────────────────────────
void MainWindow::sincronizarSprites()
{
    // ── Limpiar barras de vida variantes ──────────────────────
    for (QGraphicsRectItem* b : barrasVidaEstaticas) {
        escena->removeItem(b); delete b;
    }
    barrasVidaEstaticas.clear();

    for (QGraphicsTextItem* e : etiquetasVariantes) {
        escena->removeItem(e); delete e;
    }
    etiquetasVariantes.clear();

    // ── Efecto de golpe jugador ───────────────────────────────
    if (mostrandoGolpe) {
        contadorGolpe++;
        if (contadorGolpe >= 12) {
            mostrandoGolpe = false;
            contadorGolpe  = 0;
            spriteJugador->setPixmap(
                mirandoDerecha ? pixJugadorNormal : pixJugadorNormalIzq);
        }
    }

    // ── Animación de caminata ─────────────────────────────────
    if (jugadorMoviendose && !mostrandoGolpe) {
        contadorAnimacion++;
        if (contadorAnimacion < 10) {
            spriteJugador->setPixmap(
                mirandoDerecha ? pixJugadorNormal : pixJugadorNormalIzq);
        } else if (contadorAnimacion < 20) {
            spriteJugador->setPixmap(
                mirandoDerecha ? pixJugadorWalk : pixJugadorWalkIzq);
        } else {
            contadorAnimacion = 0;
        }
    } else if (!jugadorMoviendose && !mostrandoGolpe) {
        contadorAnimacion = 0;
        spriteJugador->setPixmap(
            mirandoDerecha ? pixJugadorNormal : pixJugadorNormalIzq);
    }

    // ── Jugador ───────────────────────────────────────────────
    Jugador* jug = engine->getJugador();
    if (jug && spriteJugador) {
        spriteJugador->setPos(jug->getX(), jug->getY());

        // 1. Orientación (Lee de la capa lógica)
        float vxJug = jug->getVelX();
        if (vxJug > 0.01f) {
            mirandoDerecha = true;
        } else if (vxJug < -0.01f) {
            mirandoDerecha = false;
        }

        // 2. Selección de Sprite por Estado
        if (mostrandoGolpe) {
            // Estado: Atacando
            spriteJugador->setPixmap(mirandoDerecha ? pixJugadorGolpe : pixJugadorGolpeIzq);

            contadorGolpe++;
            if (contadorGolpe >= 12) { // Termina la animación
                mostrandoGolpe = false;
                contadorGolpe = 0;
            }
        } else {
            // Estado: Movimiento pasivo o quieto
            bool moviendoX = std::abs(jug->getVelX()) > 0.01f;
            bool moviendoY = std::abs(jug->getVelY()) > 0.01f;

            if (moviendoX || moviendoY) {
                // Desplazándose activamente (animación de caminata)
                contadorAnimacion++;
                if (contadorAnimacion < 10) {
                    spriteJugador->setPixmap(mirandoDerecha ? pixJugadorNormal : pixJugadorNormalIzq);
                } else if (contadorAnimacion < 20) {
                    spriteJugador->setPixmap(mirandoDerecha ? pixJugadorWalk : pixJugadorWalkIzq);
                } else {
                    contadorAnimacion = 0;
                }
            } else {
                // Completamente quieto
                contadorAnimacion = 0;
                spriteJugador->setPixmap(mirandoDerecha ? pixJugadorNormal : pixJugadorNormalIzq);
            }
        }



        // Efecto de desvanecimiento suave
        float tRecJug = jug->getTiempoRecuperacionGolpe();
        if (tRecJug > 0.0f) {
            // Opacidad va de 0.3 (tras el golpe) recuperándose hasta 1.0
            spriteJugador->setOpacity(1.0f - (tRecJug / 0.5f) * 0.7f);
        } else {
            spriteJugador->setOpacity(1.0f);
        }
    }

    const QList<Enemigo*>& enemigos = engine->getEnemigos();
    int nivel = engine->getEstado().getNivel();

    if (nivel == 1) {
        // ── Nivel 1 ───────────────────────────────────────────

        // Limpiar sprites variantes por si acaso
        for (QGraphicsPixmapItem* s : spritesVariantes) {
            escena->removeItem(s); delete s;
        }
        spritesVariantes.clear();

        // Limpiar portales nivel 2 por si acaso
        for (QGraphicsPixmapItem* s : spritesPortalesNivel2) {
            escena->removeItem(s); delete s;
        }
        spritesPortalesNivel2.clear();

        // ── Angstrom Levy ──────────────────────────────────────────
        if (!enemigos.isEmpty() && spriteLevy) {
            AngstromLevy* levy = dynamic_cast<AngstromLevy*>(enemigos[0]);

            if (levy) {
                spriteLevy->setPos(levy->getX(), levy->getY());
                spriteLevy->setVisible(levy->isActivo());

                if (jug && levy->isActivo()) {
                    // 1. Orientación (Lee de la capa lógica)
                    float vxLevy = levy->getVelX();
                    bool levyMiraDerecha = true;

                    if (vxLevy > 0.01f) {
                        levyMiraDerecha = true;
                    } else if (vxLevy < -0.01f) {
                        levyMiraDerecha = false;
                    } else {
                        // Si está estático en X, por lógica siempre mira al jugador
                        levyMiraDerecha = (jug->getX() > levy->getX());
                    }

<<<<<<< HEAD
                if (levyGolpeando) {
                    contadorLevyGolpe++;
                    if (contadorLevyGolpe >= 12) {
                        levyGolpeando     = false;
=======
                    // 2. Transiciones de Estado
                    float dx = levy->getX() - jug->getX();
                    float dy = levy->getY() - jug->getY();
                    float dist = std::sqrt(dx * dx + dy * dy);

                    // Si está cerca, inicia el ataque
                    if (dist < 80.0f && !levyGolpeando) {
                        levyGolpeando = true;
>>>>>>> origin/main
                        contadorLevyGolpe = 0;
                    }

                    if (levyGolpeando) {
                        // ESTADO: ATACANDO
                        spriteLevy->setPixmap(levyMiraDerecha ? pixLevyGolpe : pixLevyGolpeIzq);

                        contadorLevyGolpe++;
                        if (contadorLevyGolpe >= 12) {
                            levyGolpeando = false;
                            contadorLevyGolpe = 0;
                        }
                    } else {
                        // ESTADO: MOVIMIENTO O QUIETO
                        bool moviendoXLevy = std::abs(vxLevy) > 0.01f;
                        bool moviendoYLevy = std::abs(levy->getVelY()) > 0.01f;

                        if (moviendoXLevy || moviendoYLevy) {
                            // ESTADO: CAMINANDO
                            // Usamos el reloj global para crear la animación (cambia de frame cada 150ms)
                            if ((reloj.elapsed() / 150) % 2 == 0) {
                                spriteLevy->setPixmap(levyMiraDerecha ? pixLevyNormal : pixLevyNormalIzq);
                            } else {
                                spriteLevy->setPixmap(levyMiraDerecha ? pixLevyWalk : pixLevyWalkIzq);
                            }
                        } else {
                            // ESTADO: BASE / QUIETO
                            spriteLevy->setPixmap(levyMiraDerecha ? pixLevyNormal : pixLevyNormalIzq);
                        }
                    }
                    float tRecLevy = levy->getTiempoRecuperacionGolpe();
                    if (tRecLevy > 0.0f) {
                        spriteLevy->setOpacity(1.0f - (tRecLevy / 0.5f) * 0.7f);
                    } else {
                        spriteLevy->setOpacity(1.0f);
                    }
                }
            }
        }

        // Portal Nivel 1 — solo 1, con animación
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

    } else {
        // ── Nivel 2 ───────────────────────────────────────────

        // Ocultar Levy y portal Nivel 1
        if (spriteLevy)  spriteLevy->setVisible(false);
        if (spritePortal) spritePortal->setVisible(false);

        // Limpiar sprites variantes
        for (QGraphicsPixmapItem* s : spritesVariantes) {
            escena->removeItem(s); delete s;
        }
        spritesVariantes.clear();

        // Limpiar portales Nivel 2
        for (QGraphicsPixmapItem* s : spritesPortalesNivel2) {
            escena->removeItem(s); delete s;
        }
        spritesPortalesNivel2.clear();

        int indexHUD = 0;
        Jugador* jug = engine->getJugador();

        for (Enemigo* e : enemigos) {
            if (!e->isActivo()) continue;

            // Sprite solo si está visible en pantalla
            if (e->isVisible()) {
                QGraphicsPixmapItem* spr = new QGraphicsPixmapItem();
                VariantePortal* vp = dynamic_cast<VariantePortal*>(e);

                if (vp) {
                    float vx = vp->getVelX();
                    float vy = vp->getVelY();

                    // 1. Orientación: sigue la velocidad horizontal;
                    //    si solo se mueve en Y, mira hacia el jugador
                    bool mirandoDer = (vx >= 0.0f);
                    if (std::abs(vx) < 0.01f && jug) {
                        mirandoDer = (jug->getX() > vp->getX());
                    }

                    // 2. Estado: ataque cuando está cerca del jugador
                    bool atacando = false;
                    if (jug) {
                        float dx   = vp->getX() - jug->getX();
                        float dy   = vp->getY() - jug->getY();
                        float dist = std::sqrt(dx * dx + dy * dy);
                        if (dist < 80.0f) atacando = true;
                    }

                    // 3. Selección de sprite
                    if (atacando) {
                        spr->setPixmap(mirandoDer ? pixVariantePortalAttack : pixVariantePortalAttackIzq);
                    } else if (std::abs(vx) > 0.01f || std::abs(vy) > 0.01f) {
                        spr->setPixmap(mirandoDer ? pixVariantePortalWalk : pixVariantePortalWalkIzq);
                    } else {
                        spr->setPixmap(pixVariantePortalBase);
                    }
                } else {
                    // Caso B: Es una variante de combate terrestre estándar (Variante)
                    Variante* v = dynamic_cast<Variante*>(e);
                    if (v) {
                        float vx = v->getVelX();
                        bool mirandoALaDerecha = true;

                        // 1. Determinar orientación horizontal (frente)
                        if (vx > 0.01f) {
                            mirandoALaDerecha = true;
                        } else if (vx < -0.01f) {
                            mirandoALaDerecha = false;
                        } else {
                            // Si está estático en X (quieto o moviéndose solo verticalmente),
                            // por diseño de juego mira siempre hacia donde esté parado el jugador
                            if (jug) {
                                mirandoALaDerecha = (jug->getX() > v->getX());
                            }
                        }

                        // 2. Asignación del Sprite según el Estado Lógico (Capa de Lógica)
                        if (v->getEstado() == Variante::Estado::ATACAR) {
                            // Estado de Ataque
                            spr->setPixmap(mirandoALaDerecha ? pixVarianteAttack : pixVarianteAttackIzq);
                        } else {
                            // Estado Rodear / Movimiento pasivo
                            if (std::abs(vx) < 0.01f) {
                                // Quieto o desplazándose ÚNICAMENTE hacia arriba o abajo
                                spr->setPixmap(pixVarianteBase);
                            } else {
                                // Desplazándose activamente de forma horizontal
                                spr->setPixmap(mirandoALaDerecha ? pixVarianteWalk : pixVarianteWalkIzq);
                            }
                        }
                    } else {
                        spr->setPixmap(pixVarianteBase);
                    }
                }
                float tRecV = e->getTiempoRecuperacionGolpe();
                if (tRecV > 0.0f) {
                    spr->setOpacity(1.0f - (tRecV / 0.5f) * 0.7f);
                } else {
                    spr->setOpacity(1.0f);
                }

                spr->setPos(e->getX(), e->getY());
                spr->setZValue(2);
                escena->addItem(spr);
                spritesVariantes.append(spr);
            }

            // HUD barra de vida
            float inicioX    = 590.0f;
            float inicioY    = 515.0f;
            float anchoBarra = 60.0f;
            float altoBarra  = 18.0f;
            float margenX    = 15.0f;
            float margenY    = 15.0f;
            int   columna    = indexHUD % 3;
            int   fila       = indexHUD / 3;
            float xPos = inicioX + columna * (anchoBarra + margenX);
            float yPos = inicioY + fila    * (altoBarra  + margenY);

            float pctVida = e->getVida() / e->getVidaMaxima();
            if (pctVida < 0.0f) pctVida = 0.0f;

            QGraphicsTextItem* etiqueta = new QGraphicsTextItem(
                "V" + QString::number(indexHUD + 1));
            etiqueta->setDefaultTextColor(Qt::white);
            etiqueta->setFont(QFont("Arial", 7, QFont::Bold));
            etiqueta->setPos(xPos, yPos + 15.0f);
            etiqueta->setZValue(12);
            escena->addItem(etiqueta);
            etiquetasVariantes.append(etiqueta);

            QGraphicsRectItem* fondoBarra = new QGraphicsRectItem(
                xPos, yPos, anchoBarra, altoBarra);
            fondoBarra->setBrush(Qt::darkRed);
            fondoBarra->setPen(Qt::NoPen);
            fondoBarra->setZValue(10);
            escena->addItem(fondoBarra);
            barrasVidaEstaticas.append(fondoBarra);

            QGraphicsRectItem* barraVida = new QGraphicsRectItem(
                xPos, yPos, anchoBarra * pctVida, altoBarra);
            barraVida->setBrush(Qt::yellow);
            barraVida->setPen(Qt::NoPen);
            barraVida->setZValue(11);
            escena->addItem(barraVida);
            barrasVidaEstaticas.append(barraVida);

            indexHUD++;
        }

        // Portales dinámicos Nivel 2
        QPixmap pixPortalN2 = QPixmap(":/sprites/assets/sprites/Portal.png")
                                  .scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        const QList<Portal*>& portales = engine->getPortales();
        for (Portal* p : portales) {
            if (!p->isActivo()) continue;
            QGraphicsPixmapItem* sprP = new QGraphicsPixmapItem(pixPortalN2);
            sprP->setPos(p->getX() - 30.0f, p->getY() - 30.0f);
            sprP->setZValue(1);
            escena->addItem(sprP);
            spritesPortalesNivel2.append(sprP);
        }
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
    int nivelActual = engine->getEstado().getNivel();

    if (victoria) {
        cambiarMusica(nivelActual == 1 ? 3 : 4);
    } else {
        cambiarMusica(nivelActual == 1 ? 5 : 6);
    }

    limpiarMenusUI();

    QGraphicsRectItem* fondo = new QGraphicsRectItem(0, 0, 800, 600);
    fondo->setBrush(QBrush(QColor(0, 0, 0, 190)));
    fondo->setZValue(10);
    escena->addItem(fondo);
    itemsMenu.append(fondo);

    // Transición al Nivel 2
    if (victoria && nivelActual == 1) {
        QGraphicsTextItem* textoExito = new QGraphicsTextItem("NIVEL 1 SUPERADO!");
        textoExito->setDefaultTextColor(QColor(0, 255, 100));
        textoExito->setFont(QFont("Arial", 40, QFont::Bold));
        textoExito->setPos(150, 180);
        textoExito->setZValue(11);
        escena->addItem(textoExito);
        itemsMenu.append(textoExito);

        QGraphicsTextItem* textoContinuar = new QGraphicsTextItem("CONTINUAR AL NIVEL 2");
        textoContinuar->setDefaultTextColor(Qt::white);
        textoContinuar->setFont(QFont("Arial", 20, QFont::Bold));
        textoContinuar->setPos(230, 300);
        textoContinuar->setZValue(11);
        escena->addItem(textoContinuar);
        itemsMenu.append(textoContinuar);

        enMenu = true;
        return;
    }

    // Victoria final o derrota
    QString mensaje  = victoria ? "VICTORIA FINAL!" : "DERROTA";
    QColor  colorMsg = victoria ? QColor(0, 255, 100) : QColor(255, 50, 50);

    QGraphicsTextItem* textoFin = new QGraphicsTextItem(mensaje);
    textoFin->setDefaultTextColor(colorMsg);
    textoFin->setFont(QFont("Arial", 48, QFont::Bold));
    textoFin->setPos(180, 180);
    textoFin->setZValue(11);
    escena->addItem(textoFin);
    itemsMenu.append(textoFin);

    QGraphicsTextItem* puntaje = new QGraphicsTextItem(
        "Puntos: " + QString::number(engine->getEstado().getPuntos()));
    puntaje->setDefaultTextColor(Qt::yellow);
    puntaje->setFont(QFont("Arial", 20));
    puntaje->setPos(320, 280);
    puntaje->setZValue(11);
    escena->addItem(puntaje);
    itemsMenu.append(puntaje);

<<<<<<< HEAD
    textoReiniciar = new QGraphicsTextItem("Presiona R para reiniciar");
    textoReiniciar->setDefaultTextColor(Qt::white);
    textoReiniciar->setFont(QFont("Arial", 16));
    textoReiniciar->setPos(230, 340);
    escena->addItem(textoReiniciar);

    textoSalir = new QGraphicsTextItem("Presiona ESC para salir");
    textoSalir->setDefaultTextColor(QColor(220, 50, 50));
    textoSalir->setFont(QFont("Arial", 16));
    textoSalir->setPos(245, 375);
    escena->addItem(textoSalir);
=======
    QGraphicsTextItem* textoRegresa = new QGraphicsTextItem("REGRESAR AL MENU");
    textoRegresa->setDefaultTextColor(Qt::white);
    textoRegresa->setFont(QFont("Arial", 20, QFont::Bold));
    textoRegresa->setPos(260, 360);
    textoRegresa->setZValue(11);
    escena->addItem(textoRegresa);
    itemsMenu.append(textoRegresa);

    enMenu = true;
>>>>>>> origin/main
}

// ── reiniciarJuego ───────────────────────────────────────────────
void MainWindow::reiniciarJuego()
{
    for (QGraphicsPixmapItem* s : spritesVariantes) {
        escena->removeItem(s); delete s;
    }
    spritesVariantes.clear();

    for (QGraphicsPixmapItem* s : spritesPortalesNivel2) {
        escena->removeItem(s); delete s;
    }
    spritesPortalesNivel2.clear();

    for (QGraphicsRectItem* b : barrasVidaEstaticas) {
        escena->removeItem(b); delete b;
    }
    barrasVidaEstaticas.clear();

<<<<<<< HEAD
    if (textoSalir) {
        escena->removeItem(textoSalir);
        delete textoSalir;
        textoSalir = nullptr;
    }

    for (QGraphicsPixmapItem* s : spritesClones) {
        escena->removeItem(s);
        delete s;
=======
    for (QGraphicsTextItem* e : etiquetasVariantes) {
        escena->removeItem(e); delete e;
>>>>>>> origin/main
    }
    etiquetasVariantes.clear();

    mirandoDerecha    = true;
    mostrandoGolpe    = false;
    contadorGolpe     = 0;
    levyGolpeando     = false;
    contadorLevyGolpe = 0;
    contadorAnimacion = 0;
    jugadorMoviendose = false;
<<<<<<< HEAD
=======
    juegoPausado      = false;
>>>>>>> origin/main

    cambiarFondo(1);
    dibujarMenuPrincipal();
    cambiarMusica(0);
    setFocus();
}

// ── crearBotonPausa ──────────────────────────────────────────────
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

// ── limpiarMenusUI ───────────────────────────────────────────────
void MainWindow::limpiarMenusUI()
{
    for (QGraphicsItem* item : itemsMenu) {
        escena->removeItem(item);
        delete item;
    }
    itemsMenu.clear();
}

// ── alternarPausa ────────────────────────────────────────────────
void MainWindow::alternarPausa()
{
    if (!juegoPausado) {
        engine->pausar();
        musicaFondo->pause();
        timerLoop->stop();
        juegoPausado = true;
        cambiarMusica(0);
        dibujarMenuPausa();
    } else {
        limpiarMenusUI();
        engine->reanudar();
        cambiarMusica(engine->getEstado().getNivel());
        musicaFondo->play();
        reloj.restart();
        timerLoop->start(16);
        juegoPausado = false;
    }
}

// ── dibujarMenuPrincipal ─────────────────────────────────────────
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

    QString opciones[] = {
        "JUGAR",
        "SELECCIONAR NIVEL (" + QString::number(nivelInicioActual) + ")",
        "SELECCIONAR DIFICULTAD"
    };
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

// ── dibujarMenuPausa ─────────────────────────────────────────────
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

    int nivelActual  = engine->getEstado().getNivel();
    int proximoNivel = (nivelActual == 1) ? 2 : 1;

    QString opciones[] = {
        "REANUDAR",
        "IR AL NIVEL " + QString::number(proximoNivel),
        "SELECCIONAR DIFICULTAD"
    };
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

// ── dibujarMenuDificultad ────────────────────────────────────────
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
    QColor  colores[]  = {
        Qt::white,
        QColor(0, 220, 100),
        QColor(255, 200, 0),
        QColor(220, 50, 50)
    };

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
