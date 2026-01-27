#pragma once

#include <QUndoCommand>
#include <QVector>
#include <QPoint>
#include "../layer/MaskLayer.h"

class MaskPaintCommand : public QUndoCommand
{
public:
    struct PixelChange
    {
        int x;
        int y;
        uchar before;
        uchar after;
    };

    MaskPaintCommand(
        MaskLayer* layer,
        QVector<PixelChange>&& changes,
        QUndoCommand* parent = nullptr
    );

    void undo() override;
    void redo() override;

private:
    MaskLayer* m_layer;
    QVector<PixelChange> m_changes;
};
