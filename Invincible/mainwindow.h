#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMap>
#include <QSet>

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

    // ── HUD ──────────────────────────────────────────────────
    QGraphicsRectItem*     barraVidaJugador;
    QGraphicsRectItem*     barraVidaLevy;
    QGraphicsTextItem*     textoPuntos;
    QGraphicsTextItem*     textoTiempo;

    // ── Input ────────────────────────────────────────────────
    QSet<int>              teclasPresionadas;

    // ── Pantalla de fin ───────────────────────────────────────
    QGraphicsTextItem*     textoFinJuego;
    QGraphicsTextItem*     textoReiniciar;

    QGraphicsTextItem*     textoPuntajeFinal; // ← agregar

    void mostrarPantallaFin(bool victoria);
    void reiniciarJuego();

    // ── Métodos privados ─────────────────────────────────────
    void inicializarEscena();
    void inicializarHUD();
    void sincronizarSprites();
    void actualizarHUD();
    void procesarInput();

};

#endif // MAINWINDOW_H
