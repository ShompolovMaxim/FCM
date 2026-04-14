#include "markdown_edit.h"

#include <QFocusEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

MarkdownEdit::MarkdownEdit(QWidget *parent) : QTextEdit(parent) {
    qApp->installEventFilter(this);
}

void MarkdownEdit::focusOutEvent(QFocusEvent *e) {
    if (_editing) {
        _markdown = toPlainText();
        setMarkdown(_markdown);
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
