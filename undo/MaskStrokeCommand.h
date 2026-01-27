#pragma once

#include <QUndoCommand>
#include <QVector>
#include <QPoint>
#include "../layer/MaskLayer.h"

class MaskStrokeCommand : public QUndoCommand {
public:
    MaskStrokeCommand(
        MaskLayer* mask,
        const QVector<QPoint>& points,
        quint8 label,
        int radius
    );

    void undo() override;
    void redo() override;

private:
    struct PixelBackup {
        QPoint pos;
        quint8 oldLabel;
    };

    MaskLayer* m_mask;
    QVector<QPoint> m_points;
    QVector<PixelBackup> m_backup;
    quint8 m_label;
    int m_radius;
};
