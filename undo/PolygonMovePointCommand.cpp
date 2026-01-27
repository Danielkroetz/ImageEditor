#include "PolygonDeletePointCommand.h"

PolygonDeletePointCommand::PolygonDeletePointCommand(EditablePolygon* poly,
                                                     int idx,
                                                     const QPointF& p,
                                                     QUndoCommand* parent)
    : AbstractCommand("Delete Polygon Point", parent)
    , m_poly(poly)
    , m_idx(idx)
    , m_point(p)
{
}

void PolygonDeletePointCommand::redo()
{
    m_poly->removePoint(m_idx);
}

void PolygonDeletePointCommand::undo()
{
    m_poly->insertPoint(m_idx, m_point);
}

QJsonObject PolygonDeletePointCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "PolygonDeletePoint";
    obj["idx"] = m_idx;
    obj["x"] = m_point.x();
    obj["y"] = m_point.y();
    return obj;
}

PolygonDeletePointCommand* PolygonDeletePointCommand::fromJson(const QJsonObject& obj,
                                                               EditablePolygon* poly)
{
    return new PolygonDeletePointCommand(
        poly,
        obj["idx"].toInt(),
        QPointF(obj["x"].toDouble(), obj["y"].toDouble())
    );
}
