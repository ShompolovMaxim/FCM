#include "adjacency_table_view.h"

#include <QMouseEvent>
#include <QModelIndex>
#include <algorithm>

AdjacencyTableView::AdjacencyTableView(QWidget* parent)
    : QTableView(parent), colorValueAdapter(std::make_unique<ColorValueAdapter>())
{
    model = new QStandardItemModel(this);
    setModel(model);
    connect(this, &QTableView::clicked, this, &AdjacencyTableView::onCellClicked);
    connect(horizontalHeader(), &QHeaderView::sectionClicked, this, &AdjacencyTableView::updateConcept);
    connect(verticalHeader(), &QHeaderView::sectionClicked, this, &AdjacencyTableView::updateConcept);
}

void AdjacencyTableView::onCellClicked(const QModelIndex& index) {
    if (idxsWeights.find(index) == idxsWeights.end()) {
        presenter->createWeight(rowsConcepts[index.row()], rowsConcepts[index.column()]);
    } else {
        presenter->updateWeight(idxsWeights[index]);
    }
}

void AdjacencyTableView::setPresenter(std::shared_ptr<CreationPresenter> newPresenter) {
    presenter = newPresenter;
    connect(presenter.get(), &CreationPresenter::conceptCreated, this, &AdjacencyTableView::conceptCreated);
    connect(presenter.get(), &CreationPresenter::conceptUpdated, this, &AdjacencyTableView::conceptUpdated);
    connect(presenter.get(), &CreationPresenter::weightCreated, this, &AdjacencyTableView::weightCreated);
    connect(presenter.get(), &CreationPresenter::weightUpdated, this, &AdjacencyTableView::weightUpdated);
    connect(presenter.get(), &CreationPresenter::conceptDeleted, this, &AdjacencyTableView::conceptDeleted);
    connect(presenter.get(), &CreationPresenter::weightDeleted, this, &AdjacencyTableView::weightDeleted);
}

void AdjacencyTableView::conceptCreated(std::shared_ptr<Concept> concept) {
    int idx = static_cast<int>(rowsConcepts.size());
    model->insertColumn(idx);
    model->setHeaderData(idx, Qt::Horizontal, concept->name);
    model->insertRow(idx);
    model->setHeaderData(idx, Qt::Vertical, concept->name);
    rowsConcepts.push_back(concept->id);
    conceptsRows[concept->id] = idx;
}

void AdjacencyTableView::weightCreated(std::shared_ptr<Weight> weight) {
    QModelIndex idx = model->index(conceptsRows[weight->fromConceptId], conceptsRows[weight->toConceptId]);
    idxsWeights[idx] = weight->id;
}

void AdjacencyTableView::conceptUpdated(std::shared_ptr<Concept> concept) {
    int idx = conceptsRows[concept->id];
    model->setHeaderData(idx, Qt::Horizontal, concept->name);
    model->setHeaderData(idx, Qt::Vertical, concept->name);
    model->setHeaderData(idx, Qt::Horizontal, colorValueAdapter->getColor(concept->value, 0, 1), Qt::BackgroundRole);
    model->setHeaderData(idx, Qt::Vertical, colorValueAdapter->getColor(concept->value, 0, 1), Qt::BackgroundRole);
}

void AdjacencyTableView::weightUpdated(std::shared_ptr<Weight> weight) {
    QModelIndex idx = model->index(conceptsRows[weight->fromConceptId], conceptsRows[weight->toConceptId]);
    model->setData(idx, weight->name);
    model->setData(idx, colorValueAdapter->getColor(weight->value, -1, 1), Qt::BackgroundRole);
}

void AdjacencyTableView::weightDeleted(size_t weightId) {
    auto it = std::find_if(idxsWeights.begin(), idxsWeights.end(), [weightId](const auto& pair){ return pair.second == weightId; });
    if (it != idxsWeights.end()) {
        model->setData(it->first, QVariant());
        model->setData(it->first, QVariant(), Qt::BackgroundRole);
        idxsWeights.erase(it);
    }
}

void AdjacencyTableView::conceptDeleted(size_t conceptId) {
    if (conceptsRows.find(conceptId) == conceptsRows.end()) return;
    int idx = conceptsRows[conceptId];
    model->removeRow(idx);
    model->removeColumn(idx);
    conceptsRows.erase(conceptId);
    rowsConcepts.erase(rowsConcepts.begin() + idx);
    for (size_t i = idx; i < rowsConcepts.size(); ++i) {
        conceptsRows[rowsConcepts[i]] = static_cast<int>(i);
    }
}

void AdjacencyTableView::updateConcept(int idx) {
    presenter->updateConcept(idx);
}
