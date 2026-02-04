#include "MoveLayerCommand.h"

#include <QPainter>
#include <QtMath>
#include <QDebug>

// -------------- Undo / redo  -------------- 
void MoveLayerCommand::undo() 
{ 
  qDebug() << "void MoveLayerCommand::undo(): old_pos =" << m_oldPos;
  if ( m_layer ) m_layer->setPos(m_oldPos);
  else qWarning() << "MoveLayerCommand::undo(): Invalid layer";
}

void MoveLayerCommand::redo()
{ 
  qDebug() << "MoveLayerCommand::redo(): old_pos= " << m_oldPos << " -> new_pos =" << m_newPos;
  if ( m_layer ) m_layer->setPos(m_newPos); 
  else qWarning() << "MoveLayerCommand::redo(): Invalid layer";
}

// -------------- history related methods  -------------- 
QJsonObject MoveLayerCommand::toJson() const
{
   QJsonObject obj = AbstractCommand::toJson();
   obj["layerId"] = m_layerId;
   obj["fromX"] = m_oldPos.x();
   obj["fromY"] = m_oldPos.y();
   obj["toX"] = m_newPos.x();
   obj["toY"] = m_newPos.y();
   obj["type"] = type();
   return obj;
}

MoveLayerCommand* MoveLayerCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers )
{
    const int layerId = obj["layerId"].toInt(-1);
    LayerItem* layer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == layerId ) {
            layer = l;
            break;
        }
    }
    if ( !layer ) {
      qWarning() << "MoveLayerCommand::fromJson(): Layer " << layerId << " not found.";
      return nullptr;
    }
    return new MoveLayerCommand(
        layer,
        QPointF(obj["fromX"].toDouble(), obj["fromY"].toDouble()),
        QPointF(obj["toX"].toDouble(),   obj["toY"].toDouble()),
        obj["layerId"].toInt()
    );
}

