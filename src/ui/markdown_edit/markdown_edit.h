#include <QTextEdit>

class MarkdownEdit : public QTextEdit {
    Q_OBJECT

public:
    using QTextEdit::QTextEdit;

protected:
    void focusOutEvent(QFocusEvent *e) override {
        _markdown = toPlainText();

        setReadOnly(true);
        setMarkdown(_markdown);

        QTextEdit::focusOutEvent(e);
    }

    void focusInEvent(QFocusEvent *e) override {
        setReadOnly(false);

        setPlainText(_markdown);

        QTextEdit::focusInEvent(e);
    }

private:
    QString _markdown;
};
