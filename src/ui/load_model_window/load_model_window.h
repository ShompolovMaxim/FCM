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
    explicit LoadModelWindow(QStringList modelsNames, const QString &windowTitle, QWidget *parent = nullptr);
    ~LoadModelWindow();

    QString selectedModelName() const;

signals:
    void deleteModelRequested(const QString &modelName);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void onModelDeleted(const QString &modelName, bool success);

private:
    void onDeleteButtonClicked();
    void removeModel(const QString &modelName);
    void addModelItem(const QString &modelName);
    void updateItemsSizeHints();
    void updateButtonsState();

    Ui::LoadModelWindow *ui;
};

#endif // LOAD_MODEL_WINDOW_H
