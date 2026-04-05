#include <QTest>
#include <QString>

class ExampleQtUiTest : public QObject
{
    Q_OBJECT

private slots:
    void smokeTest() {
        QString text = "ui test template";
        QCOMPARE(text.toUpper(), QString("UI TEST TEMPLATE"));
    }
};

QTEST_APPLESS_MAIN(ExampleQtUiTest)

#include "example_qt_ui_test.moc"
