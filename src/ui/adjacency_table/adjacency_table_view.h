#pragma once

#include "ui/graph_editor/color_value_adapter/color_value_adapter.h"

#include "presenter/creation_presenter.h"

#include <QStandardItemModel>
#include <QTableView>

#include <memory>

class AdjacencyTableView : public QTableView
{
    Q_OBJECT

public:
    explicit AdjacencyTableView(QWidget* parent = nullptr);

    void setPresenter(std::shared_ptr<CreationPresenter> newPresenter);

private slots:
    void onCellClicked(const QModelIndex& index);

    void conceptCreated(std::shared_ptr<Concept> concept);
    void weightCreated(std::shared_ptr<Weight> weight);
    void conceptUpdated(std::shared_ptr<Concept> concept);
    void weightUpdated(std::shared_ptr<Weight> weight);
    void conceptDeleted(size_t id);
    void weightDeleted(size_t id);

    void updateConcept(int idx);

private:
    std::shared_ptr<CreationPresenter> presenter;
    std::unique_ptr<ColorValueAdapter> colorValueAdapter;
    QStandardItemModel* model;
    std::map<size_t, size_t> conceptsRows;
    std::vector<size_t> rowsConcepts;
    std::map<QModelIndex, size_t> idxsWeights;
};
