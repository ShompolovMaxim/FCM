#pragma once

#include <QObject>
#include <QUuid>
#include <QTreeWidget>
#include <memory>

class FCM;
class QWidget;
class CreationPresenter;
class StaticAnalysisPresenter;
class SimulationScenePresenter;

namespace Ui {
class MainWindow;
}

class ModelSetupPresenter : public QObject {
    Q_OBJECT
public:
    ModelSetupPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<CreationPresenter>& creationPresenter, StaticAnalysisPresenter*& staticAnalysisPresenter, std::shared_ptr<SimulationScenePresenter>& presenter, QWidget* parentWidget, QObject* parent = nullptr);

    bool checkElementsHaveValues();
    void descriptionChanged();

    void onCreateTerm();
    void onDeleteTerm();
    void onChooseTermColor();
    void onCurrentItemChanged(QTreeWidgetItem  *current, QTreeWidgetItem *previous);
    void onTermValueChanged(double value);
    void onTermValueLChanged(double value);
    void onTermValueMChanged(double value);
    void onTermValueUChanged(double value);
    void onItemChanged(QTreeWidgetItem  *item, int column);
    void termNotesChanged();

private:
    void updateFuzzyValuePlot();
    void autoConfigureTermColor();
    void autoConfigureNumericValue();
    void autoConfigureFuzzyValue();
    void popagateTermUpdate();

    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<FCM>& fcm;
    QUuid currentTermId;
    QTreeWidgetItem* conceptsGroup;
    QTreeWidgetItem* weightsGroup;
    std::shared_ptr<CreationPresenter>& creationPresenter;
    StaticAnalysisPresenter*& staticAnalysisPresenter;
    std::shared_ptr<SimulationScenePresenter>& presenter;
};
