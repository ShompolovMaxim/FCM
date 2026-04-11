#ifndef JOIN_WINDOW_H
#define JOIN_WINDOW_H

#include "group_type.h"
#include "selection_mode.h"

#include "model/join/mode.h"

#include <QDialog>

namespace Ui {
class JoinWindow;
}

class JoinWindow : public QDialog {
    Q_OBJECT

public:
    explicit JoinWindow(const QList<QString>& unsavedModelsNames, const QList<QString>& savedModelsNames, const QList<QString>& templatesNames, QWidget *parent = nullptr);
    ~JoinWindow();

    const QMap<JoinGroupType, QList<QString>>& getModelsToJoin() const;
    QString getTermsModel() const;
    QString getResultName() const;
    JoinMode getJoinMode() const;

private slots:
    void filterTree(const QString &text);
    void next();
    void back();
    void onOkClicked();

private:
    void populateTree(const QList<QList<QString>>& modelsNames, const QList<QString>& headers);
    QString getSelectedName() const;
    QMap<JoinGroupType, QList<QString>> getSelectedNames() const;

    Ui::JoinWindow *ui;
    QPushButton* backButton;
    QPushButton* nextButton;
    QPushButton* okButton;

    const QList<QString>& unsavedModelsNames;
    const QList<QString>& savedModelsNames;
    const QList<QString>& templatesNames;
    QList<QString> modelsToJoin;
    SelectionMode selectionMode;
    QMap<JoinGroupType, QList<QString>> selectedNames;
    QString termsModel;
};

#endif // JOIN_WINDOW_H
