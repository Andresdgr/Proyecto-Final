#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "nivel2.h" // Asegúrate de incluir el nivel

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Crear una escena que será administrada por la ventana
    QGraphicsScene* escenaPrincipal = new QGraphicsScene(this);

    // 2. Instanciar Nivel 2 pasándole únicamente la escena
    nivelActual = new Nivel2(escenaPrincipal);

    // 3. Vincular la escena al visor usando el getter que acabamos de crear
    ui->graphicsView->setScene(nivelActual->getEscena());

    // 4. Inicializar el escenario (el jugador ahora se crea internamente aquí)
    nivelActual->inicializarEscenario();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete nivelActual;
}
