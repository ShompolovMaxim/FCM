#include "ui/main_window/main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");
    a.setPalette(QApplication::style()->standardPalette());
    qApp->setStyleSheet(
        "QLineEdit, QTextEdit, QListWidget {"
        "border: 1px solid palette(mid);"
        "}"
        );

    MainWindow w;
    w.showMaximized();
    //w.show();

    return a.exec();
}
