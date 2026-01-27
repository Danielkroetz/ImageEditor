#pragma once

#include <QUndoCommand>
#include <QVector>
#include <QPointF>

class LayerItem;

class CageMoveCommand : public QUndoCommand
{
public:
    CageMoveCommand(
        LayerItem* layer,
        const QVector<QPointF>& before,
        const QVector<QPointF>& after
    );

    void undo() override;
    void redo() override;

private:
    LayerItem* m_layer;
    QVector<QPointF> m_before;
    QVector<QPointF> m_after;
};
