#include "help_browser.h"

#include <QHelpEngine>
#include <QImage>
#include <QTextDocument>
#include <QTextEdit>

HelpBrowser::HelpBrowser(QWidget *parent) : QTextBrowser(parent) {
    setLineWrapMode(QTextEdit::WidgetWidth);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void HelpBrowser::setHelpEngine(QHelpEngine *engine) {
    helpEngine = engine;
}

QVariant HelpBrowser::loadResource(int type, const QUrl &name) {
    QUrl resolvedUrl = name;
    if (resolvedUrl.isRelative()) {
        resolvedUrl = source().resolved(resolvedUrl);
    }

    if (resolvedUrl.scheme() == "qthelp" && helpEngine) {
        const QByteArray data = helpEngine->fileData(resolvedUrl);
        if (type == QTextDocument::StyleSheetResource) {
            return QString::fromUtf8(data);
        }
        if (type == QTextDocument::ImageResource) {
            QImage image;
            if (image.loadFromData(data)) {
                image.setDevicePixelRatio(devicePixelRatioF());
                return image;
            }
        }
        return data;
    }

    return QTextBrowser::loadResource(type, resolvedUrl);
}
