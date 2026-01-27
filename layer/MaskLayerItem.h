#pragma once

#include <QGraphicsItem>
#include <QImage>
#include "MaskLayer.h"

class MaskLayerItem : public QGraphicsItem {

public:
    explicit MaskLayerItem( MaskLayer* layer );

    void setOpacityFactor( qreal o );
    void setLabelColors( const QVector<QColor>& colors );

    QRectF boundingRect() const override;
    QColor labelColor( int index ) {
      return m_labelColors.at(index);
    }
    void maskUpdated();
    
protected:
    void paint( QPainter* p, const QStyleOptionGraphicsItem*, QWidget* ) override;

private:
    MaskLayer* m_layer = nullptr;
    qreal m_opacityFactor = 0.4;
    QImage m_cachedImage; 
    bool m_dirty = true;
    QVector<QColor> m_labelColors; // size 10
    
};