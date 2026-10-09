//#include "widgets/test.h"
#include "widgets/appmain.h"
#include <QFontDatabase>
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Leacesy_Ryan");
    QApplication::setOverrideCursor(Qt::BlankCursor);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); -* Warning: It will deform.

    int fontId = QFontDatabase::addApplicationFont(":/fonts/wqy-microhei.ttc");
    if (fontId != -1) {
        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            app.setFont(QFont(families.at(0)));
        }
    }

    /*test tw;
    tw.show();
    return app.exec();*/

    //QTimer::singleShot(9000, &app, &QGuiApplication::quit); // 9s
    Mainwindow mw; // inside -> show
    return app.exec();
}

