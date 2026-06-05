#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QSet>
#include <QList>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>

#include "gameengine.h"
#include "difficultyconfig.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void onUpdate();

private:

    Ui::MainWindow*        ui;

    // ── Escena y vista ───────────────────────────────────────
    QGraphicsScene*        escena;
    QGraphicsView*         vista;

    // ── Game loop ────────────────────────────────────────────
    QTimer*                timerLoop;
    QElapsedTimer          reloj;

    // ── Configuración y lógica ───────────────────────────────
    DifficultyConfig       config;
    GameEngine*            engine;

    // ── Sprites ──────────────────────────────────────────────
    QGraphicsPixmapItem*   spriteJugador;
    QGraphicsPixmapItem*   spriteLevy;
    QGraphicsPixmapItem*   spritePortal;
    QGraphicsPixmapItem*   itemFondo;
    QList<QGraphicsPixmapItem*> spritesClones;

    // ── Pixmaps jugador ───────────────────────────────────────
    QPixmap                pixJugadorNormal;
    QPixmap                pixJugadorNormalIzq;
    QPixmap                pixJugadorGolpe;
    QPixmap                pixJugadorGolpeIzq;
    bool                   mostrandoGolpe;
    int                    contadorGolpe;
    bool                   mirandoDerecha;

    // ── Pixmaps Levy ──────────────────────────────────────────
    QPixmap                pixLevyNormal;
    QPixmap                pixLevyNormalIzq;
    QPixmap                pixLevyGolpe;
    QPixmap                pixLevyGolpeIzq;
    bool                   levyGolpeando;
    int                    contadorLevyGolpe;

    // ── HUD ──────────────────────────────────────────────────
    QGraphicsRectItem*     barraVidaJugador;
    QGraphicsRectItem*     barraVidaLevy;
    QGraphicsRectItem*     fondoVidaLevy;
    QGraphicsTextItem*     labelLevy;
    QGraphicsTextItem*     textoPuntos;
    QGraphicsTextItem*     textoTiempo;

    // ── Pantalla de fin ───────────────────────────────────────
    QGraphicsTextItem*     textoFinJuego;
    QGraphicsTextItem*     textoReiniciar;
    QGraphicsTextItem*     textoPuntajeFinal;

    // ── Menú ──────────────────────────────────────────────────
    QGraphicsRectItem*     fondoMenu;
    QGraphicsTextItem*     textoTitulo;
    QGraphicsTextItem*     textoSubtitulo;
    QGraphicsTextItem*     textoFacil;
    QGraphicsTextItem*     textoNormal;
    QGraphicsTextItem*     textoDificil;
    bool                   enMenu;

    // ── Audio ─────────────────────────────────────────────────
    QMediaPlayer*          musicaFondo;
    QAudioOutput*          audioFondo;
    QSoundEffect*          efectoGolpe;
    QSoundEffect*          efectoVictoria;
    QSoundEffect*          efectoDerrota;

    // ── Input ────────────────────────────────────────────────
    QSet<int>              teclasPresionadas;

    // ── Métodos privados ─────────────────────────────────────
    void inicializarEscena();
    void inicializarHUD();
    void sincronizarSprites();
    void actualizarHUD();
    void procesarInput();
    void mostrarPantallaFin(bool victoria);
    void reiniciarJuego();
    void mostrarMenu();
    void ocultarMenu();
    void cambiarFondo(int nivel);
    void cambiarMusica(int nivel);
};

#endif // MAINWINDOW_H
