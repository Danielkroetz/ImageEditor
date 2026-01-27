#include "MaskStrokeCommand.h"
#include <QtMath>

MaskStrokeCommand::MaskStrokeCommand(
    MaskLayer* mask,
    const QVector<QPoint>& points,
    quint8 label,
    int radius
)
    : m_mask(mask),
      m_points(points),
      m_label(label),
      m_radius(radius)
{
    setText("Mask Stroke");

    for (const QPoint& p : points) {
        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                if (x*x + y*y > radius*radius) continue;

                QPoint pt = p + QPoint(x,y);
                if (pt.x() < 0 || pt.y() < 0 ||
                    pt.x() >= mask->width() || pt.y() >= mask->height())
                    continue;

                m_backup.push_back({pt, mask->labelAt(pt.x(), pt.y())});
            }
        }
    }
}

void MaskStrokeCommand::redo() {
    for (auto& b : m_backup)
        m_mask->setLabelAt(b.pos.x(), b.pos.y(), m_label);
}

void MaskStrokeCommand::undo() {
    for (auto& b : m_backup)
        m_mask->setLabelAt(b.pos.x(), b.pos.y(), b.oldLabel);
}
