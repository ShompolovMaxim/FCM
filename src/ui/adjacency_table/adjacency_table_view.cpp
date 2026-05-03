#include "adjacency_table_view.h"

#include <QMouseEvent>
#include <QModelIndex>
#include <algorithm>

AdjacencyTableView::AdjacencyTableView(QWidget* parent) : QTableView(parent), colorValueAdapter(std::make_unique<ColorValueAdapter>()) {
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
        presenter->updateWeight(idxsWeights[index], ElementWindowMode::UpdateElement);
    }
}

void AdjacencyTableView::setPresenter(std::shared_ptr<CreationPresenter> newPresenter) {
    if (presenter) {
        disconnect(presenter.get(), nullptr, this, nullptr);
    }
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
    //resizeColumnsToContents();
}

void AdjacencyTableView::weightCreated(std::shared_ptr<Weight> weight) {
    QModelIndex idx = model->index(conceptsRows[weight->fromConceptId], conceptsRows[weight->toConceptId]);
    idxsWeights[idx] = weight->id;
}

void AdjacencyTableView::conceptUpdated(std::shared_ptr<Concept> concept) {
    int idx = conceptsRows[concept->id];
    model->setHeaderData(idx, Qt::Horizontal, concept->name);
    model->setHeaderData(idx, Qt::Vertical, concept->name);
    auto color = QColor(255, 255, 255);
    if (concept->term) {
        color = colorValueAdapter->getColor(concept->term->value, 0, 1);
    }
    model->setHeaderData(idx, Qt::Horizontal, color, Qt::BackgroundRole);
    model->setHeaderData(idx, Qt::Vertical, color, Qt::BackgroundRole);
    resizeColumnsToContents();
}

void AdjacencyTableView::weightUpdated(std::shared_ptr<Weight> weight) {
    QModelIndex idx = model->index(conceptsRows[weight->fromConceptId], conceptsRows[weight->toConceptId]);
    model->setData(idx, weight->name);
    auto color = QColor(255, 255, 255);
    if (weight->term) {
        color = colorValueAdapter->getColor(weight->term->value, -1, 1);
    }
    model->setData(idx, color, Qt::BackgroundRole);
}

void AdjacencyTableView::weightDeleted(QUuid weightId) {
    auto it = std::find_if(idxsWeights.begin(), idxsWeights.end(), [weightId](const auto& pair){ return pair.second == weightId; });
    if (it != idxsWeights.end()) {
        model->setData(it->first, QVariant());
        model->setData(it->first, QVariant(), Qt::BackgroundRole);
        idxsWeights.erase(it);
    }
}

void AdjacencyTableView::conceptDeleted(QUuid conceptId) {
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
    presenter->updateConcept(rowsConcepts[idx], ElementWindowMode::UpdateElement);
}

void AdjacencyTableView::loadFromFCM(const std::shared_ptr<FCM>& fcm) {
    model->clear();
    rowsConcepts.clear();
    conceptsRows.clear();
    idxsWeights.clear();

    for (const auto& [id, concept] : fcm->concepts) {
        conceptCreated(concept);
    }
    for (const auto& [id, concept] : fcm->concepts) {
        conceptUpdated(concept);
    }
    for (const auto& [id, weight] : fcm->weights) {
        weightCreated(weight);
    }
    for (const auto& [id, weight] : fcm->weights) {
        weightUpdated(weight);
    }
}
