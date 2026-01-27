#pragma once

#include <QGraphicsPixmapItem>
#include <QImage>
#include "EditablePolygon.h"

class PolygonMaskLayerItem : public QGraphicsPixmapItem
{
public:
    PolygonMaskLayerItem( EditablePolygon* poly, const QImage& baseImage );

    void updateMask();               // wendet Polygon auf baseImage an
    void setOpacity( float opacity );  // Layer über original legen

    QImage maskImage() const { return m_mask; }
    EditablePolygon* polygon() const { return m_poly; }

private:
    EditablePolygon* m_poly;
    QImage m_baseImage; // Ausgangsbild
    QImage m_mask;      // Maskenbild, transparent außerhalb
    float m_opacity = 1.0f;
};
