#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <map>
#include <memory>

#include "model/entities/fcm.h"
#include "presenter/simulation_presenter.h"
#include "ui/graph_editor/edit_mode.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateGraphScaleLabel(double newScale);
    void updatePredictScaleLabel(double newScale);
    void updateModeButtonText(EditMode newMode);
    void predict();
    void resetPredictionScene();

    void onListWidgetContextMenu(const QPoint &pos);
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onTermValueChanged(double value);
    void onTermValueLChanged(double value);
    void onTermValueMChanged(double value);
    void onTermValueUChanged(double value);
    void onItemChanged(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    SimulationPresenter presenter = SimulationPresenter(nullptr);
    std::map<QListWidgetItem*, Term> terms;
    std::shared_ptr<FCM> fcm;
    QListWidgetItem* currentTerm;

};
#endif // MAIN_WINDOW_H
