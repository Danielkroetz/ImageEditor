#include "CageOverlayItem.h"
#include "LayerItem.h"
#include "CageMesh.h"
#include <QPainter>

CageOverlayItem::CageOverlayItem(LayerItem* layer)
    : m_layer(layer)
{
    setZValue(10000);          // immer über dem Bild
    setAcceptedMouseButtons(Qt::NoButton);
}

QRectF CageOverlayItem::boundingRect() const
{
    return m_layer->boundingRect();
}

void CageOverlayItem::paint(QPainter* p,
                            const QStyleOptionGraphicsItem*,
                            QWidget*)
{
    const CageMesh& mesh = m_layer->cageMesh();
    if ( mesh.pointCount() == 0 )
        return;
    QPen pen(QColor(0, 255, 0));
    pen.setWidth(0); // cosmetic
    p->setPen(pen);
    int cols = mesh.cols();
    int rows = mesh.rows();
    const auto& pts = mesh.points();
    // horizontale Linien
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x + 1 < cols; ++x) {
            int i1 = y * cols + x;
            int i2 = i1 + 1;
            p->drawLine(pts[i1], pts[i2]);
        }
    }
    // vertikale Linien
    for (int x = 0; x < cols; ++x) {
        for (int y = 0; y + 1 < rows; ++y) {
            int i1 = y * cols + x;
            int i2 = i1 + cols;
            p->drawLine(pts[i1], pts[i2]);
        }
    }
}
