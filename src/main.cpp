#include "ui/main_window/main_window.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");
    a.setPalette(QApplication::style()->standardPalette());
    qApp->setStyleSheet(
        "QLineEdit, QTextEdit, QListWidget {"
        "border: 1px solid palette(mid);"
        "}"
        );

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "FCM_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            //a.installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
