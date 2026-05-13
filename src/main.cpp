#include "ui/main_window/main_window.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[]) {
    const QSettings settings("app.ini", QSettings::IniFormat);

    if (settings.value("ui/disableDarkMode", true).toBool()) {
        qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    }

    QApplication a(argc, argv);
    QApplication::setStyle(settings.value("ui/style", "Fusion").toString());
    a.setPalette(QApplication::style()->standardPalette());
    qApp->setStyleSheet(settings.value(
        "ui/globalStyleSheet",
        "QLineEdit, QTextEdit, QListWidget {"
        "border: 1px solid palette(mid);"
        "}"
    ).toString());
    MainWindow w;
    w.showMaximized();
    //w.show();

    return a.exec();
}
