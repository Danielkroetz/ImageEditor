#include "PolygonReduceCommand.h"

// ---------------------------- Constructor ----------------------------
PolygonReduceCommand::PolygonReduceCommand( EditablePolygon* poly, QUndoCommand* parent )
    : AbstractCommand(parent),
      m_poly(poly)
{
  m_before = m_poly->polygon();
  setText(QString("Reduce polygon"));
}

// ----------------------------  ----------------------------
void PolygonReduceCommand::undo()
{
    if ( !m_poly )
      return; 
    m_poly->setPolygon(m_before);  
}

void PolygonReduceCommand::redo()
{
    if ( !m_poly )
      return; 
    m_poly->reduce();
}

// ----------------------------  ---------------------------- 
QJsonObject PolygonReduceCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "PolygonReduce";
    obj["layerId"] = 0;
    QJsonArray pts;
    for ( const QPointF& p : m_before ) {
        QJsonObject jp;
        jp["x"] = p.x();
        jp["y"] = p.y();
        pts.append(jp);
    }
    obj["points"] = pts;
    return obj;
}

PolygonReduceCommand* PolygonReduceCommand::fromJson( const QJsonObject& obj, EditablePolygon* poly )
{
    if ( poly == nullptr ) 
      return nullptr;
    const int layerId = obj["layerId"].toInt(-1);
    QPolygonF polygon;
    QJsonArray pts = obj["points"].toArray();
    for ( const QJsonValue& v : pts ) {
        QJsonObject jp = v.toObject();
        polygon << QPointF(jp["x"].toDouble(), jp["y"].toDouble());
    }
    poly->setPolygon(polygon);
    return new PolygonReduceCommand(poly);
}