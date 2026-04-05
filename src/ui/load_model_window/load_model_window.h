#ifndef LOAD_MODEL_WINDOW_H
#define LOAD_MODEL_WINDOW_H

#include <QDialog>

namespace Ui {
class LoadModelWindow;
}

class LoadModelWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoadModelWindow(QStringList modelsNames, QWidget *parent = nullptr);
    ~LoadModelWindow();

    QString selectedModelName() const;

signals:
    void deleteModel(const QString &modelName);

private:
    Ui::LoadModelWindow *ui;
};

#endif // LOAD_MODEL_WINDOW_H
