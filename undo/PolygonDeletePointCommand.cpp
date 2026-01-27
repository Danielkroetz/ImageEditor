#include "PolygonDeletePointCommand.h"

PolygonDeletePointCommand::PolygonDeletePointCommand(
    EditablePolygon* poly, int idx, const QPointF& p)
    : AbstractCommand("Delete Polygon Point"),
      m_poly(poly), m_idx(idx), m_point(p)
{}

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
    QJsonObject o = AbstractCommand::toJson();
    o["type"] = "PolygonDeletePoint";
    o["idx"] = m_idx;
    o["x"] = m_point.x();
    o["y"] = m_point.y();
    return o;
}
