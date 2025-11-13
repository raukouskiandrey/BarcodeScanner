#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Barcode Scanner");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("YourCompany");

    MainWindow window;
    window.show();

    return app.exec();
}
