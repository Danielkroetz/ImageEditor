#pragma once

#include <QGraphicsItem>

class LayerItem;

class CageOverlayItem : public QGraphicsItem
{
public:
    explicit CageOverlayItem(LayerItem* layer);

    QRectF boundingRect() const override;
    void paint(QPainter* p,
               const QStyleOptionGraphicsItem*,
               QWidget*) override;

private:
    LayerItem* m_layer;
};
