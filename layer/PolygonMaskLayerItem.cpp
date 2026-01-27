#include "PolygonMaskLayerItem.h"
#include <QPainter>
#include <QBrush>

PolygonMaskLayerItem::PolygonMaskLayerItem(EditablePolygon* poly,
                                           const QImage& baseImage)
    : m_poly(poly)
    , m_baseImage(baseImage)
{
    setZValue(100); // über dem Original
    updateMask();

    connect(m_poly, &EditablePolygon::changed,
            this, &PolygonMaskLayerItem::updateMask);
}

void PolygonMaskLayerItem::setOpacity( float opacity )
{
    m_opacity = qBound(0.0f, opacity, 1.0f);
    setOpacity(m_opacity);
}

void PolygonMaskLayerItem::updateMask()
{
    if (m_baseImage.isNull() || !m_poly) return;

    QImage img(m_baseImage.size(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QBrush(m_baseImage));
    p.setPen(Qt::NoPen);

    // Polygon in Bild-Koordinaten
    QPolygonF polyScene = m_poly->polygon();
    p.drawPolygon(polyScene);

    p.end();

    m_mask = img;
    setPixmap(QPixmap::fromImage(m_mask));
    setOpacity(m_opacity);
}

QJsonObject toJson() const
{
    QJsonObject obj;
    obj["type"] = "PolygonMaskLayer";
    obj["polygon"] = m_poly->toJson();
    obj["opacity"] = m_opacity;
    return obj;
}

static PolygonMaskLayerItem* fromJson( const QJsonObject& obj, const QImage& baseImage )
{
    EditablePolygon* poly = EditablePolygon::fromJson(obj["polygon"].toObject());
    PolygonMaskLayerItem* item = new PolygonMaskLayerItem(poly, baseImage);
    item->setOpacity(obj["opacity"].toDouble(1.0));
    return item;
}