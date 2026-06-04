#include <QApplication>
#include "MainController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    app.setApplicationName("SnapTranslate");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("SnapTranslate");

    MainController controller;
    controller.start();

    return app.exec();
}
