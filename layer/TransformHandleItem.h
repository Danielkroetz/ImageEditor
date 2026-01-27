#pragma once

#include <QGraphicsEllipseItem>

class LayerItem;
class QUndoStack;

class TransformHandleItem : public QGraphicsEllipseItem
{
public:

    enum Role { Scale, Rotate };

    TransformHandleItem(LayerItem* layer, Role role);

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;

private:

    LayerItem* m_layer;
    Role m_role;
    QPointF m_pressScenePos;
    QTransform m_startTransform;

};