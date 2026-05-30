#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Inicializa el entorno gráfico de Qt
    MainWindow w;               // Al crearse, se ejecuta el constructor de MainWindow
    w.show();                   // Hace visible la ventana en la pantalla
    return a.exec();            // Entra en el "bucle de eventos" (mantiene vivo el programa)
}
