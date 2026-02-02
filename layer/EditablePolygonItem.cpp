#include "EditablePolygonItem.h"

#include "../undo/PolygonTranslateCommand.h"
#include "../undo/PolygonDeletePointCommand.h"
#include "../undo/PolygonInsertPointCommand.h"
#include "../undo/PolygonMovePointCommand.h"
#include "../undo/PolygonReduceCommand.h"
#include "../undo/PolygonSmoothCommand.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

#include <iostream>

// --------------------------------- Constructor ---------------------------------
EditablePolygonItem::EditablePolygonItem( EditablePolygon* poly, QGraphicsItem* parent )
    : QGraphicsObject(parent)
    , m_poly(poly)
{
    Q_ASSERT(m_poly);
    m_layer = dynamic_cast<LayerItem*>(parent);
    Q_ASSERT(m_layer);
    setZValue(1000);
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    connect(m_poly, &EditablePolygon::changed, this, &EditablePolygonItem::updateGeometry);
    connect(m_poly, &EditablePolygon::visibilityChanged, this, &EditablePolygonItem::onVisibilityChanged);
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
    if ( !m_poly->polygonVisible() )
        return;
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
  std::cout << "EditablePolygonItem::mousePressEvent(): m_editable=" << m_editable << std::endl;
  {
    if ( !m_editable )
        return;
    if ( m_layer != nullptr ) {
      LayerItem::OperationMode mode = m_layer->operationMode();
      if ( mode == LayerItem::OperationMode::TranslatePolygon ) {
       m_dragStartPos = e->scenePos();
       m_dragMousePressPos = m_dragStartPos;
       e->accept();
       return;
      } else {
       m_activePoint = hitTestPoint(e->scenePos());
       if ( m_activePoint >= 0 ) {
          m_dragStartPos = m_poly->point(m_activePoint);
          e->accept();
          return;
       }
      }
    } else {
     std::cout << "EditablePolygonItem::mousePressEvent(): No layer available." << std::endl;
    }
    QGraphicsObject::mousePressEvent(e);
  }
}

void EditablePolygonItem::mouseMoveEvent( QGraphicsSceneMouseEvent* e )
{
  // std::cout << "EditablePolygonItem::mouseMoveEvent(): Processing..." << std::endl;
  {
    if ( m_layer == nullptr )
      return;
    LayerItem::OperationMode mode = m_layer->operationMode();
    if ( mode == LayerItem::OperationMode::MovePoint ) {
      if ( m_activePoint >= 0 ) {
        pointMoved(m_activePoint, e->scenePos());
        e->accept();
        return;
      }
    } else if ( mode == LayerItem::OperationMode::TranslatePolygon ) {
     QPointF delta = e->scenePos()-m_dragStartPos;
     m_poly->translate(delta);
     m_dragStartPos = e->scenePos();
    }
    QGraphicsObject::mouseMoveEvent(e);
  }
}

void EditablePolygonItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* e )
{
    if ( m_layer == nullptr )
      return;
    LayerItem::OperationMode mode = m_layer->operationMode();
    if ( mode == LayerItem::OperationMode::MovePoint ) {
      if ( m_activePoint >= 0 ) {
        QPointF endPos = m_poly->point(m_activePoint);
        m_poly->undoStack()->push(new PolygonMovePointCommand(m_poly,m_activePoint,m_dragStartPos,endPos));
        // emit requestMovePoint(m_activePoint,m_dragStartPos,endPos);
        m_activePoint = -1;
        e->accept();
        return;
      }
    } else if ( mode == LayerItem::OperationMode::TranslatePolygon ) {
        m_poly->undoStack()->push(new PolygonTranslateCommand(m_poly,m_dragMousePressPos,e->scenePos()));
        e->accept();
        return;
    }
    QGraphicsObject::mouseReleaseEvent(e);
}

void EditablePolygonItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* e )
{
 qDebug() << "EditablePolygonItem::mouseDoubleClickEvent(): opMode =" << m_layer->operationMode() << ", id =" << m_layer->id() << ", name ="  << m_layer->name();
 {
    LayerItem::OperationMode mode = m_layer->operationMode();
    if ( mode == LayerItem::OperationMode::AddPoint ) {
     int edge = hitTestEdge(e->scenePos());
     if ( edge >= 0 ) {
        // emit requestInsertPoint(edge + 1, e->scenePos());
        m_poly->undoStack()->push(new PolygonInsertPointCommand(m_poly,edge+1,e->scenePos()));
        e->accept();
        return;
     }
    } else if ( mode == LayerItem::OperationMode::DeletePoint ) {
     int pointId = hitTestPoint(e->scenePos());
     if ( pointId >= 0 ) {
        m_poly->undoStack()->push(new PolygonDeletePointCommand(m_poly,pointId,m_poly->point(pointId)));
        e->accept();
        return;
     }
    } else if ( mode == LayerItem::OperationMode::Reduce ) {
     m_poly->undoStack()->push(new PolygonReduceCommand(m_poly));
     e->accept();
     return;
    } else if ( mode == LayerItem::OperationMode::Smooth ) {
     m_poly->undoStack()->push(new PolygonSmoothCommand(m_poly));
     e->accept();
     return;
    } else if ( mode == LayerItem::OperationMode::DeletePolygon ) {
     m_poly->remove();
     e->accept();
     return;
    } else if ( mode == LayerItem::OperationMode::Info ) {
     m_poly->printself();
     return;
    }
    QGraphicsObject::mouseDoubleClickEvent(e);
 }
}

// ---------------- Editing helpers ----------------

void EditablePolygonItem::pointMoved( int idx, const QPointF& scenePos )
{
    if ( idx < 0 || idx >= m_poly->pointCount() )
        return;
    m_poly->setPoint(idx, scenePos);
}

int EditablePolygonItem::hitTestPoint( const QPointF& scenePos ) const
{
    for ( int i = 0; i < m_handles.size(); ++i ) {
        if ( QLineF(m_handles[i]->scenePos(), scenePos).length()
            <= m_handleRadius * 2.0 )
            return i;
    }
    return -1;
}

int EditablePolygonItem::hitTestEdge( const QPointF& scenePos ) const
{
    const QPolygonF& poly = m_poly->polygon();
    for ( int i = 0; i < poly.size(); ++i ) {
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

void EditablePolygonItem::onVisibilityChanged()
{
    // Polygon
    setVisible(m_poly->polygonVisible());
    // Marker (Control Points)
    for ( auto* p : m_handles )
        p->setVisible(m_poly->markersVisible());
}

void EditablePolygonItem::rebuildHandles()
{
    qDeleteAll(m_handles);
    m_handles.clear();

    const QPolygonF& poly = m_poly->polygon();
    for ( const QPointF& p : poly ) {
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
