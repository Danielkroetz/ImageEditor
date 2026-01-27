#include "CageControlPointItem.h"
#include "LayerItem.h"

#include <QGraphicsSceneMouseEvent>
#include <iostream>

CageControlPointItem::CageControlPointItem( LayerItem* layer, int index )
    : QGraphicsRectItem(-2, -2, 4, 4),
      m_layer(layer), m_index(index)
{
	setBrush(Qt::red);
    setZValue(10001);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsScenePositionChanges);
}

void CageControlPointItem::mousePressEvent( QGraphicsSceneMouseEvent *e )
{
    // std::cout << "CageControlPointItem::mousePressEvent((): Processing..." << std::endl;
    // m_startPoint = m_layer->mapFromScene(e->scenePos());
    // m_layer->beginCageEdit();
}

void CageControlPointItem::mouseMoveEvent( QGraphicsSceneMouseEvent* e )
{
    // std::cout << "CageControlPointItem::mouseMoveEvent((): Processing..." << std::endl;
    m_layer->setCagePoint(m_index, e->scenePos());
    e->accept();
    
    // QPointF scenePos = e->scenePos();
    // QPointF layerLocalPos = m_layer->mapFromScene(scenePos);
    // setPos(layerLocalPos);
    // m_layer->setCagePoint(m_index, layerLocalPos);
    // e->accept();
}

void CageControlPointItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* e )
{
  // std::cout << "CageControlPointItem::mouseReleaseEvent((): Processing..." << std::endl;
    // m_layer->endCageEdit(m_index,m_startPoint);
    e->accept();
}
