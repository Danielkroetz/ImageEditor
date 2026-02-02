#include "PolygonTranslateCommand.h"

#include <iostream>

// ---------------------------- Constructor ----------------------------
PolygonTranslateCommand::PolygonTranslateCommand( EditablePolygon* poly, 
           const QPointF& start, const QPointF& end, QUndoCommand* parent )
    : AbstractCommand(parent)
      , m_poly(poly)
      , m_start(start)
      , m_end(end)
{
  setText(QString("Translate polygon"));
  m_redo = false;
}

// ----------------------------  ----------------------------
void PolygonTranslateCommand::undo()
{
    QPointF delta = m_start-m_end; 
    m_poly->translate(delta);
}

void PolygonTranslateCommand::redo()
{
    if ( m_redo == false ) {
      m_redo = true;
      return;
    }
    QPointF delta = m_end-m_start; 
    m_poly->translate(delta);
}

// ----------------------------  ---------------------------- 
QJsonObject PolygonTranslateCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "PolygonTranslate";
    obj["x_start"] = m_start.x();
    obj["y_start"] = m_start.y();
    obj["x_end"] = m_end.x();
    obj["y_end"] = m_end.y();
    return obj;
}

PolygonTranslateCommand* PolygonTranslateCommand::fromJson( const QJsonObject& obj, EditablePolygon* poly )
{
    QPointF start = QPointF(obj["x_start"].toDouble(),obj["y_start"].toDouble());
    QPointF end = QPointF(obj["x_end"].toDouble(),obj["y_end"].toDouble());
    return new PolygonTranslateCommand(poly,start,end);
}