#include "CageLayerItem.h"
#include "CageControlPointItem.h"
#include <QPainter>

#include "../undo/TransformLayerCommand.h"

CageLayerItem::CageLayerItem( const QPixmap& pix, const QPolygonF& cagePolygon ) : LayerItem(pix)
{
    if ( cagePolygon.isEmpty() ) {
        createDefaultBoundingBox();
    } else {
        m_cage = cagePolygon;
    }
    for ( int i = 0; i < m_cage.size(); ++i ) {
        auto* p = new CageControlPointItem(this, i);
        p->setParentItem(this);
        p->setRect(-4, -4, 8, 8);
        p->setBrush(Qt::cyan);
        p->setPen(QPen(Qt::black, 0));
        p->setFlag(ItemIgnoresTransformations);
        p->setZValue(3000);
        p->setPos(m_cage[i]);
        m_points << p;
    }
}

void CageLayerItem::createDefaultBoundingBox()
{
    QRectF br = boundingRect();
    m_cage.clear();
    m_cage << br.topLeft()
           << br.topRight()
           << br.bottomRight()
           << br.bottomLeft();
}

void CageLayerItem::disableCage()
{
    if ( !m_cageEnabled )
        return;
    m_cageEnabled = false;
    // removeCagePoints();
}

bool CageLayerItem::cageEnabled() const
{
    return m_cageEnabled;
}

void CageLayerItem::beginCageEdit()
{
    m_editing = true;
    m_startPos = pos();
    m_startTransform = transform();
}

void CageLayerItem::endCageEdit()
{
    m_editing = false;
    if ( undoStack() && m_startTransform != transform() )
        undoStack()->push(new TransformLayerCommand(this, m_startPos, pos(), m_startTransform, transform()));
}

void CageLayerItem::setCagePoint( int idx, const QPointF& pos )
{
    if ( idx < 0 || idx >= m_cage.size() ) return;
    m_cage[idx] = pos;
    // Transformation: Mapping vom Originalpolygon auf neues Polygon
    if ( m_cage.size() >= 4 ) { // Mindestens 4 Punkte für Quad-Transform
        QTransform t;
        QTransform::quadToQuad(
            QVector<QPointF>{
                boundingRect().topLeft(),
                boundingRect().topRight(),
                boundingRect().bottomRight(),
                boundingRect().bottomLeft()
            },
            m_cage.mid(0,4), // erste 4 Punkte für affine Approx
            t
        );
        setTransform(t);
    }
    // Kontrollpunkt-Visual aktualisieren
    m_points[idx]->setPos(pos);
}

void CageLayerItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
    LayerItem::paint(p, o, w);
    p->setPen(QPen(Qt::cyan, 0, Qt::DashLine));
    p->drawPolygon(QPolygonF(m_cage));
}
