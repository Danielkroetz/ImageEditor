#include "EditablePolygonItem.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

EditablePolygonItem::EditablePolygonItem( EditablePolygon* poly, QGraphicsItem* parent )
    : QGraphicsObject(parent)
    , m_poly(poly)
{
    Q_ASSERT(m_poly);

    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);

    connect(m_poly, &EditablePolygon::changed,
            this, &EditablePolygonItem::updateGeometry);

    rebuildHandles();
}

QRectF EditablePolygonItem::boundingRect() const
{
    QRectF r = m_poly->boundingRect();
    r.adjust(-10, -10, 10, 10); // Platz für Handles
    return r;
}

void EditablePolygonItem::paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* )
{
    const QPolygonF& poly = m_poly->polygon();
    if ( poly.size() < 2 )
        return;
    p->setRenderHint(QPainter::Antialiasing);
    // Fill
    p->setBrush(m_fillColor);
    p->setPen(QPen(m_lineColor, 1.5));
    p->drawPolygon(poly);
    // Outline
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(m_lineColor, 2));
    p->drawPolyline(poly);
}

// ---------------- Interaction ----------------

void EditablePolygonItem::mousePressEvent( QGraphicsSceneMouseEvent* e )
{
    if ( !m_editable )
        return;
    m_activePoint = hitTestPoint(e->scenePos());
    if (m_activePoint >= 0) {
        m_dragStartPos = m_poly->point(m_activePoint);
        e->accept();
        return;
    }
    QGraphicsObject::mousePressEvent(e);
}

void EditablePolygonItem::mouseMoveEvent( QGraphicsSceneMouseEvent* e )
{
    if ( m_activePoint >= 0 ) {
        pointMoved(m_activePoint, e->scenePos());
        e->accept();
        return;
    }
    QGraphicsObject::mouseMoveEvent(e);
}

void EditablePolygonItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* e )
{
    if ( m_activePoint >= 0 ) {
        QPointF endPos = m_poly->point(m_activePoint);
        emit requestMovePoint(m_activePoint,
                              m_dragStartPos,
                              endPos);
        m_activePoint = -1;
        e->accept();
        return;
    }

    QGraphicsObject::mouseReleaseEvent(e);
}

void EditablePolygonItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* e )
{
    int edge = hitTestEdge(e->scenePos());
    if ( edge >= 0 ) {
        emit requestInsertPoint(edge + 1, e->scenePos());
        e->accept();
        return;
    }

    QGraphicsObject::mouseDoubleClickEvent(e);
}

// ---------------- Editing helpers ----------------

void EditablePolygonItem::pointMoved( int idx, const QPointF& scenePos )
{
    if ( idx < 0 || idx >= m_poly->pointCount() )
        return;
    // direkte visuelle Rückmeldung
    m_poly->setPoint(idx, scenePos);
}

int EditablePolygonItem::hitTestPoint( const QPointF& scenePos ) const
{
    for (int i = 0; i < m_handles.size(); ++i) {
        if (QLineF(m_handles[i]->scenePos(), scenePos).length()
            <= m_handleRadius * 2.0)
            return i;
    }
    return -1;
}

int EditablePolygonItem::hitTestEdge( const QPointF& scenePos ) const
{
    const QPolygonF& poly = m_poly->polygon();
    for (int i = 0; i < poly.size(); ++i) {
        QPointF a = poly[i];
        QPointF b = poly[(i + 1) % poly.size()];
        QLineF l(a, b);
        if ( l.length() < 1e-3 )
            continue;
        qreal d = QLineF(a, scenePos).length()
                + QLineF(scenePos, b).length()
                - l.length();
        if ( qAbs(d) < 3.0 )
            return i;
    }
    return -1;
}

// ---------------- Geometry sync ----------------

void EditablePolygonItem::updateGeometry()
{
    prepareGeometryChange();
    rebuildHandles();
    update();
}

void EditablePolygonItem::rebuildHandles()
{
    qDeleteAll(m_handles);
    m_handles.clear();

    const QPolygonF& poly = m_poly->polygon();
    for (const QPointF& p : poly) {
        auto* h = new QGraphicsEllipseItem(
            -m_handleRadius,
            -m_handleRadius,
            2 * m_handleRadius,
            2 * m_handleRadius,
            this);

        h->setBrush(m_handleColor);
        h->setPen(Qt::NoPen);
        h->setPos(p);
        h->setZValue(10);

        m_handles.push_back(h);
    }
}
