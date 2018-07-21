// TensionWiz Version 2.1.0b
// Matti Syrjanen

//#include "vld.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const QString filePath = argv[1];

    MainWindow w(filePath);
    w.show();

    return a.exec();
}


