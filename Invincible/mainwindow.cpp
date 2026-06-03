#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gameengine.h"
#include "difficultyconfig.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Configurar el tamaño estricto de la ventana principal
    this->setFixedSize(800, 600);

    // 2. Hacer que el QGraphicsView ocupe todo el espacio de la ventana sin márgenes
    this->setCentralWidget(ui->graphicsView);

    // 3. Configurar el visor (graphicsView) para no mostrar barras ni bordes
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setFrameShape(QFrame::NoFrame); // Evita un borde gris de 1 pixel

    // 4. Crear una escena que será administrada por la ventana
    QGraphicsScene* escenaPrincipal = new QGraphicsScene(this);

    // Forzar la escena matemática a ser exactamente de 800x600
    escenaPrincipal->setSceneRect(0, 0, 800, 600);

    // 5. Vincular la escena al visor de la interfaz gráfica
    ui->graphicsView->setScene(escenaPrincipal);

    // 6. Instanciar el GameEngine pasándole la escena principal
    motorJuego = new GameEngine(DifficultyConfig::normal());


    // 7. Inicializar el escenario
    motorJuego->iniciarNivel(2, escenaPrincipal);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete motorJuego;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    // Bloquear el auto-repeat para la barra espaciadora
    if (event->isAutoRepeat() && event->key() == Qt::Key_Space) {
        return;
    }
    // Si el motor ya está inicializado, le pasamos la tecla
    if (motorJuego) {
        motorJuego->teclaPresionada(event->key());
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    // Bloquear el auto-repeat  para la barra espaciadora
    if (event->isAutoRepeat() && event->key() == Qt::Key_Space) {
        return;
    }
    if (motorJuego) {
        motorJuego->teclaSoltada(event->key());
    }
}
