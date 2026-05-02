#include <QApplication>
#include "views/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Установка стиля приложения (опционально)
    app.setStyle("Fusion");
    
    // Создание и показ главного окна
    MainWindow window;
    window.show();
    
    return app.exec();
}
