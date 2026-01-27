#pragma once

#include <QGraphicsRectItem>

class LayerItem;

class CageControlPointItem : public QGraphicsRectItem
{

public:
    CageControlPointItem( LayerItem* layer, int index );

protected:
    void mousePressEvent( QGraphicsSceneMouseEvent* ) override;
    void mouseMoveEvent( QGraphicsSceneMouseEvent* ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* ) override;

private:
    LayerItem* m_layer;
    QPointF m_startPoint;
    int m_index;

};
