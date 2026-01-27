#include "EditablePolygon.h"

EditablePolygon::EditablePolygon(QObject* parent)
    : QObject(parent)
{
}

QPointF EditablePolygon::point(int idx) const
{
    if (idx < 0 || idx >= m_polygon.size())
        return {};
    return m_polygon[idx];
}

void EditablePolygon::setPoint(int idx, const QPointF& p)
{
    if (idx < 0 || idx >= m_polygon.size())
        return;

    m_polygon[idx] = p;
    emit changed();
}

void EditablePolygon::insertPoint(int idx, const QPointF& p)
{
    if (idx < 0 || idx > m_polygon.size())
        return;

    m_polygon.insert(idx, p);
    emit changed();
}

void EditablePolygon::removePoint(int idx)
{
    if (idx < 0 || idx >= m_polygon.size())
        return;

    m_polygon.removeAt(idx);
    emit changed();
}

void EditablePolygon::setPolygon(const QPolygonF& poly)
{
    m_polygon = poly;
    emit changed();
}

QRectF EditablePolygon::boundingRect() const
{
    return m_polygon.boundingRect();
}

// ---------------- Serialization ----------------

QJsonObject EditablePolygon::toJson() const
{
    QJsonObject obj;
    QJsonArray arr;

    for (const QPointF& p : m_polygon) {
        QJsonObject po;
        po["x"] = p.x();
        po["y"] = p.y();
        arr.append(po);
    }

    obj["points"] = arr;
    return obj;
}

EditablePolygon* EditablePolygon::fromJson(const QJsonObject& obj)
{
    auto* poly = new EditablePolygon;
    QPolygonF polygon;

    QJsonArray arr = obj["points"].toArray();
    for (const QJsonValue& v : arr) {
        QJsonObject po = v.toObject();
        polygon << QPointF(po["x"].toDouble(),
                            po["y"].toDouble());
    }

    poly->setPolygon(polygon);
    return poly;
}
