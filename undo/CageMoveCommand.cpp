#include "CageMoveCommand.h"
#include "../layer/LayerItem.h"

CageMoveCommand::CageMoveCommand(
    LayerItem* layer,
    const QVector<QPointF>& before,
    const QVector<QPointF>& after)
    : m_layer(layer), m_before(before), m_after(after)
{
    setText("Cage Deformation");
}

void CageMoveCommand::undo()
{
    m_layer->setCagePoints(m_before);
    m_layer->applyCageWarp();
}

void CageMoveCommand::redo()
{
    m_layer->setCagePoints(m_after);
    m_layer->applyCageWarp();
}
