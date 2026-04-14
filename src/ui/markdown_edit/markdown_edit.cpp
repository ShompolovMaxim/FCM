#include "markdown_edit.h"

#include <QFocusEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QTimer>

MarkdownEdit::MarkdownEdit(QWidget *parent) : QTextEdit(parent) {
    qApp->installEventFilter(this);
}

void MarkdownEdit::setMarkdownText(const QString& text) {
    _markdown = text;

    if (_editing) {
        setPlainText(_markdown);
    } else {
        setMarkdown(_markdown);
    }
}

QString MarkdownEdit::markdownText() const {
    if (_editing) {
        return toPlainText();
    }
    return _markdown;
}

void MarkdownEdit::focusOutEvent(QFocusEvent *e) {
    if (_editing) {
        _markdown = toPlainText();
        QTimer::singleShot(0, this, [this]() {
            setMarkdown(_markdown);
        });
        setReadOnly(true);
        _editing = false;
    }
    QTextEdit::focusOutEvent(e);
}

void MarkdownEdit::focusInEvent(QFocusEvent *e) {
    if (!_editing) {
        setPlainText(_markdown);
        setReadOnly(false);
        _editing = true;
    }
    QTextEdit::focusInEvent(e);
}

bool MarkdownEdit::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QWidget *w = QApplication::widgetAt(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        if (!w || (!isAncestorOf(w) && w != this)) {
            clearFocus();
        }
    }
    return QTextEdit::eventFilter(obj, event);
}

void MarkdownEdit::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Return && (e->modifiers() & Qt::ControlModifier)) {
        clearFocus();
        e->accept();
        return;
    }

    QTextEdit::keyPressEvent(e);
}
