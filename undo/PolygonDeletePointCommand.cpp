#include "PolygonDeletePointCommand.h"

// --------------------- Constructor ---------------------
PolygonDeletePointCommand::PolygonDeletePointCommand(
    EditablePolygon* poly, int idx, const QPointF& p, QUndoCommand* parent )
    : AbstractCommand(parent)
      , m_poly(poly)
      , m_idx(idx)
      , m_point(p)
{
  setText(QString("Delete polygon point %1").arg(idx));
}

// ---------------------  ---------------------
void PolygonDeletePointCommand::redo()
{
    m_poly->removePoint(m_idx);
}

void PolygonDeletePointCommand::undo()
{
    m_poly->insertPoint(m_idx, m_point);
}

// ---------------------  ---------------------
QJsonObject PolygonDeletePointCommand::toJson() const
{
    QJsonObject o = AbstractCommand::toJson();
    o["type"] = "PolygonDeletePoint";
    o["idx"] = m_idx;
    o["x"] = m_point.x();
    o["y"] = m_point.y();
    return o;
}

PolygonDeletePointCommand* PolygonDeletePointCommand::fromJson( const QJsonObject& obj, EditablePolygon* poly )
{
    return new PolygonDeletePointCommand(
        poly,
        obj["idx"].toInt(),
        QPointF(obj["x"].toDouble(), obj["y"].toDouble())
    );
}