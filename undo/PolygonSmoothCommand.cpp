#include "PolygonSmoothCommand.h"

// ---------------------------- Constructor ----------------------------
PolygonSmoothCommand::PolygonSmoothCommand( EditablePolygon* poly, QUndoCommand* parent )
    : AbstractCommand(parent),
      m_poly(poly)
{
  m_before = m_poly->polygon();
  setText(QString("Smooth polygon"));
}

// ----------------------------  ----------------------------
void PolygonSmoothCommand::undo()
{
    if ( !m_poly )
      return; 
    m_poly->setPolygon(m_before);  
}

void PolygonSmoothCommand::redo()
{
    if ( !m_poly )
      return; 
    m_poly->smooth();
}

// ----------------------------  ---------------------------- 
QJsonObject PolygonSmoothCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "PolygonSmooth";
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

PolygonSmoothCommand* PolygonSmoothCommand::fromJson( const QJsonObject& obj, EditablePolygon* poly )
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
    return new PolygonSmoothCommand(poly);
}