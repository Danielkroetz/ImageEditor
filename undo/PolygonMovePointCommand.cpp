#include "PolygonMovePointCommand.h"

PolygonMovePointCommand::PolygonMovePointCommand(
    EditablePolygon* poly, int idx, const QPointF& o, const QPointF& n,
    QUndoCommand* parent )
    : AbstractCommand(parent),
      m_poly(poly), m_idx(idx), m_oldPos(o), m_newPos(n)
{
  setText(QString("Move polygon point %1").arg(idx));
}

void PolygonMovePointCommand::undo()
{
    m_poly->setPoint(m_idx, m_oldPos);
}

void PolygonMovePointCommand::redo()
{
    m_poly->setPoint(m_idx, m_newPos);
}

QJsonObject PolygonMovePointCommand::toJson() const
{
    QJsonObject o = AbstractCommand::toJson();
    o["type"] = "PolygonMovePoint";
    o["idx"] = m_idx;
    o["ox"] = m_oldPos.x();
    o["oy"] = m_oldPos.y();
    o["nx"] = m_newPos.x();
    o["ny"] = m_newPos.y();
    return o;
}

PolygonMovePointCommand* PolygonMovePointCommand::fromJson(
    const QJsonObject& o, EditablePolygon* poly)
{
    return new PolygonMovePointCommand(
        poly,
        o["idx"].toInt(),
        QPointF(o["ox"].toDouble(), o["oy"].toDouble()),
        QPointF(o["nx"].toDouble(), o["ny"].toDouble())
    );
}
