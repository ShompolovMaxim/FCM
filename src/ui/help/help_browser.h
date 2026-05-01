#pragma once

#include <QTextBrowser>

class QHelpEngine;

class HelpBrowser : public QTextBrowser {
    Q_OBJECT

public:
    explicit HelpBrowser(QWidget *parent = nullptr);

    void setHelpEngine(QHelpEngine *engine);

protected:
    QVariant loadResource(int type, const QUrl &name) override;

private:
    QHelpEngine *helpEngine = nullptr;
};
