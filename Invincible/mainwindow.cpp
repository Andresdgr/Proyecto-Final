#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gameengine.h"
#include "difficultyconfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Crear una escena que será administrada por la ventana
    QGraphicsScene* escenaPrincipal = new QGraphicsScene(this);

    // 2. Vincular la escena al visor de la interfaz gráfica
    ui->graphicsView->setScene(escenaPrincipal);

    // 3. Instanciar el GameEngine pasándole la escena principal
    motorJuego = new GameEngine(DifficultyConfig::normal());

    // 4. Inicializar el escenario
    motorJuego->iniciarNivel(2, escenaPrincipal);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete motorJuego;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    // Si el motor ya está inicializado, le pasamos la tecla
    if (motorJuego) {
        motorJuego->teclaPresionada(event->key());
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (motorJuego) {
        motorJuego->teclaSoltada(event->key());
    }
}
