#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "model/entities/fcm.h"

class QWidget;
class QSettings;
class TemplatesManager;
class ModelsSavingManager;

class ModelsJoinPresenter : public QObject {
    Q_OBJECT
public:
    ModelsJoinPresenter(
        std::vector<std::shared_ptr<FCM>>& fcms,
        std::shared_ptr<TemplatesManager> templatesManager,
        std::shared_ptr<ModelsSavingManager> savingManager,
        QSettings& settings,
        QWidget* parentWidget,
        QObject *parent = nullptr
    );

    void joinModels();

signals:
    void addFCMRequested(std::shared_ptr<FCM> fcm);
    void loadFCMRequested(std::shared_ptr<FCM> fcm);

private:
    std::vector<std::shared_ptr<FCM>>& fcms;

    std::shared_ptr<TemplatesManager> templatesManager;
    std::shared_ptr<ModelsSavingManager> savingManager;
    QWidget* parentWidget;

    QSettings& settings;
};

