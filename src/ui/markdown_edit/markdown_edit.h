#pragma once

#include <QTextEdit>

class MarkdownEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit MarkdownEdit(QWidget *parent = nullptr);

    void setMarkdownText(const QString& text);
    QString markdownText() const;

protected:
    void focusOutEvent(QFocusEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void keyPressEvent(QKeyEvent *e) override;

private:
    QString _markdown;
    bool _editing = false;

    using QTextEdit::setPlainText;
    using QTextEdit::toPlainText;
};
