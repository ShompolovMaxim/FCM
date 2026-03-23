#pragma once
#include <QListWidgetItem>

class TermListItem : public QListWidgetItem {
public:
    TermListItem(const QString& text, size_t termId = 0) : QListWidgetItem(text), termId(termId) {}

    void setTermId(size_t id) { termId = id; }
    size_t getTermId() const { return termId; }

private:
    size_t termId;
};
