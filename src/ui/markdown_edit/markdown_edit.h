#pragma once

#include <QTextEdit>

class MarkdownEdit : public QTextEdit {
    Q_OBJECT

public:
    explicit MarkdownEdit(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QString _markdown;
    bool _editing = true;
};
