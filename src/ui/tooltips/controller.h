#pragma once

#include <QObject>
#include <QEvent>

class ToolTipController : public QObject {
    Q_OBJECT

public:
    explicit ToolTipController(QObject *parent = nullptr);

    void setEnabled(bool on);
    bool isEnabled() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool enabled;
};
