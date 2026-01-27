#include "MaskPaintCommand.h"

MaskPaintCommand::MaskPaintCommand(
    MaskLayer* layer,
    QVector<PixelChange>&& changes,
    QUndoCommand* parent
)
    : QUndoCommand(parent)
    , m_layer(layer)
    , m_changes(std::move(changes))
{
    setText(QString("Mask paint (%1 px)").arg(m_changes.size()));
}

void MaskPaintCommand::undo()
{
    if ( !m_layer ) return;
    for (const auto& c : m_changes)
        m_layer->setPixel(c.x, c.y, c.before);

    m_layer->emitChanged();
}

void MaskPaintCommand::redo()
{
    if ( !m_layer ) return;
    for (const auto& c : m_changes)
        m_layer->setPixel(c.x, c.y, c.after);
    m_layer->emitChanged();
}
