#pragma once

#include <QObject>
#include <QUuid>
#include <QTreeWidget>
#include <memory>

#include "model/prediction/prediction_parameters.h"
#include "ui/graph_editor/edit_mode.h"

class FCM;
class QWidget;
class CreationPresenter;
class QKeyEvent;

namespace Ui {
class MainWindow;
}

class ModelSetupPresenter : public QObject {
    Q_OBJECT
public:
    ModelSetupPresenter(Ui::MainWindow* ui, std::shared_ptr<FCM>& fcm, std::shared_ptr<CreationPresenter>& creationPresenter, QWidget* parentWidget, QObject* parent = nullptr);

    std::shared_ptr<FCM> currentModel() const { return fcm; }
    bool keyPressEvent(QKeyEvent* event);
    void reconfigure();
    void retranslateUi();
    void updateFCM();
    void updateGraphScaleLabel(double newScale);
    void updateModeButtonText(EditMode newMode);
    void descriptionChanged();

    void cancelPendingWeightCreation();
    void onCreateTerm();
    void onDeleteTerm();
    void onChooseTermColor();
    void onCurrentItemChanged(QTreeWidgetItem  *current, QTreeWidgetItem *previous);
    void onCurrentTabChanged(int index);
    void onTermValueChanged(double value);
    void onTermValueLChanged(double value);
    void onTermValueMChanged(double value);
    void onTermValueUChanged(double value);
    void onItemChanged(QTreeWidgetItem  *item, int column);
    void termNotesChanged();

signals:
    void popagateTermUpdate();

private:
    PredictionParameters getPredictionParameters() const;
    void updatePredictionParameters();
    void updateFuzzyValuePlot();
    void autoConfigureTermColor();
    void autoConfigureNumericValue();
    void autoConfigureFuzzyValue();

    Ui::MainWindow* ui;
    QWidget* parentWidget;
    std::shared_ptr<FCM>& fcm;
    QUuid currentTermId;
    QTreeWidgetItem* conceptsGroup;
    QTreeWidgetItem* weightsGroup;
    std::shared_ptr<CreationPresenter>& creationPresenter;
    double graphScale = 1.0;
    EditMode editMode = EditMode::Create;
};
