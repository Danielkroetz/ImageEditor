#include "PolygonInsertPointCommand.h"

PolygonInsertPointCommand::PolygonInsertPointCommand( EditablePolygon* poly,
                                                     int idx,
                                                     const QPointF& p,
                                                     QUndoCommand* parent )
    : AbstractCommand(parent)
    , m_poly(poly)
    , m_idx(idx)
    , m_point(p)
{
  setText(QString("Insert polygon point at %1").arg(idx));
}

void PolygonInsertPointCommand::redo()
{
    m_poly->insertPoint(m_idx, m_point);
}

void PolygonInsertPointCommand::undo()
{
    m_poly->removePoint(m_idx);
}

QJsonObject PolygonInsertPointCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "PolygonInsertPoint";
    obj["idx"] = m_idx;
    obj["x"] = m_point.x();
    obj["y"] = m_point.y();
    return obj;
}

PolygonInsertPointCommand* PolygonInsertPointCommand::fromJson( const QJsonObject& obj, EditablePolygon* poly )
{
    return new PolygonInsertPointCommand(
        poly,
        obj["idx"].toInt(),
        QPointF(obj["x"].toDouble(), obj["y"].toDouble())
    );
}
