#pragma once

#include <QGraphicsObject>
#include <QPolygonF>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

class EditablePolygonItem;

/**
 * @brief Logisches Polygon (Model)
 * Wird von Commands verändert und von EditablePolygonItem dargestellt
 */
class EditablePolygon : public QObject
{
    Q_OBJECT
public:
    explicit EditablePolygon(QObject* parent = nullptr);

    // --- Zugriff ---
    const QPolygonF& polygon() const { return m_polygon; }
    int pointCount() const { return m_polygon.size(); }
    QPointF point(int idx) const;

    // --- Modifikation (NUR über Commands aufrufen!) ---
    void setPoint(int idx, const QPointF& p);
    void insertPoint(int idx, const QPointF& p);
    void removePoint(int idx);
    void setPolygon(const QPolygonF& poly);

    QRectF boundingRect() const;

    // --- Serialization ---
    QJsonObject toJson() const;
    static EditablePolygon* fromJson(const QJsonObject& obj);

signals:
    void changed();

private:
    QPolygonF m_polygon;
};

