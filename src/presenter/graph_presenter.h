#pragma once

#include "edit_mode.h"

#include "model/entities/fcm.h"

#include "ui/graph_editor/edge_item.h"
#include "ui/graph_editor/node_item.h"

#include <map>
#include <memory>

class GraphPresenter
{
public:
    GraphPresenter(std::shared_ptr<FCM> fcm);


private:
    EditMode editMode = EditMode::Create;
    std::shared_ptr<FCM> fcm;
    std::map<EdgeItem*, size_t> edgeIds;
    std::map<NodeItem*, size_t> nodeIds;
};
