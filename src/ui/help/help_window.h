#pragma once

#include <QDialog>
#include <QHelpEngine>

class QUrl;

namespace Ui {
class HelpWindow;
}

class HelpWindow : public QDialog {
    Q_OBJECT

public:
    explicit HelpWindow(QWidget *parent = nullptr);
    ~HelpWindow();

    void retranslate();

private:
    void reloadHelpEngine();
    QString currentLanguageCode() const;
    QString currentNamespace() const;
    QUrl defaultPageUrl() const;
    void showHelpPage(const QUrl &url);

    Ui::HelpWindow *ui;
    QHelpEngine *helpEngine;
};
