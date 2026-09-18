#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

/*
int main(int argc, char *argv[])
{
    // QApplication a(argc, argv);
    // a.setWindowIcon(QIcon(":/resources/icons/256x256/Picture_Duplicates.png"));

    // MainWindow w;
}
*/

int main(int argc, char *argv[])
{
    // -------------------------------------------------------------------------------------------
    QString version = "v1.0.2";  // Current version of this program !!!!!!!!!!!!!!!!!!!!!!
    // -------------------------------------------------------------------------------------------

    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];

        if (arg == "--version") {
            std::cout << "Version: " << version.toStdString() << std::endl;
            return 0;
        }
    }

    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("Picture_Duplicates");

    a.setWindowIcon(QIcon(":/resources/icons/256x256/Picture_Duplicates.png"));

//    QTranslator translator;
//    translator.load("app_de.qm");
//    a.installTranslator(&translator);

    MainWindow w(version);
    w.show();
    w.setWindowIcon(QIcon(":/resources/icons/Picture_Duplicates.svg"));
    return a.exec();
}

