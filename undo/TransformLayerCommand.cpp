#include "TransformLayerCommand.h"

#include "../layer/LayerItem.h"
#include <iostream>

// -------------------------------- Constructor --------------------------------
TransformLayerCommand::TransformLayerCommand( LayerItem* layer,
    const QPointF& oldPos, const QPointF& newPos, const QTransform& oldTransform,
    const QTransform& newTransform, const QString& name, QUndoCommand* parent )
    : AbstractCommand(parent),
      m_layer(layer), 
      m_oldPos(oldPos), 
      m_newPos(newPos), 
      m_oldTransform(oldTransform), 
      m_newTransform(newTransform),
      m_name(name)
{
    m_layerId = layer->id();
    setText(name);
}

// -------------------------------- Undo/Redo --------------------------------
void TransformLayerCommand::undo() {
  qDebug() << "TransformLayerCommand::undo(): m_oldTransform =" << m_oldTransform;
  {
    if ( m_layer )
      m_layer->setImageTransform(m_oldTransform);
  }
}

void TransformLayerCommand::redo() {
  qDebug() << "TransformLayerCommand::redo(): m_newTransform =" << m_newTransform;
  {
    if ( m_layer )
      m_layer->setImageTransform(m_newTransform);
  }
}

// -------------- JSON stuff -------------- 
QJsonObject TransformLayerCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    
    // core
    obj["type"] = "TransformLayerCommand";
    obj["layerId"] = m_layer ? m_layer->id() : -1;
    obj["name"] = m_name;
    
    // points
    QJsonObject oldPointObj;
    oldPointObj["x"] = m_oldPos.x();
    oldPointObj["y"] = m_oldPos.y();
    obj["oldPosition"] = oldPointObj;
    QJsonObject newPointObj;
    newPointObj["x"] = m_newPos.x();
    newPointObj["y"] = m_newPos.y();
    obj["newPosition"] = newPointObj;
    
    // transforms
    QJsonObject oldTransformObj;
    oldTransformObj["m11"] = m_oldTransform.m11(); oldTransformObj["m12"] = m_oldTransform.m12(); oldTransformObj["m13"] = m_oldTransform.m13();
    oldTransformObj["m21"] = m_oldTransform.m21(); oldTransformObj["m22"] = m_oldTransform.m22(); oldTransformObj["m23"] = m_oldTransform.m23();
    oldTransformObj["m31"] = m_oldTransform.m31(); oldTransformObj["m32"] = m_oldTransform.m32(); oldTransformObj["m33"] = m_oldTransform.m33();
    obj["oldTransform"] = oldTransformObj;
    QJsonObject newTransformObj;
    newTransformObj["m11"] = m_newTransform.m11(); newTransformObj["m12"] = m_newTransform.m12(); newTransformObj["m13"] = m_newTransform.m13();
    newTransformObj["m21"] = m_newTransform.m21(); newTransformObj["m22"] = m_newTransform.m22(); newTransformObj["m23"] = m_newTransform.m23();
    newTransformObj["m31"] = m_newTransform.m31(); newTransformObj["m32"] = m_newTransform.m32(); newTransformObj["m33"] = m_newTransform.m33();
    obj["newTransform"] = newTransformObj;
    

    return obj; 
}

TransformLayerCommand* TransformLayerCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent )
{
    // Layer
    const int layerId = obj["layerId"].toInt(-1);
    LayerItem* layer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == layerId ) {
            layer = l;
            break;
        }
    }
    if ( !layer ) {
        qWarning() << "TransformLayerCommand::fromJson(): Layer not found:" << layerId;
        return nullptr;
    }
    
    // core
    QString name = obj.value("name").toString("Unknown");
    
    // points
    QJsonObject oldPointObj = obj["oldPosition"].toObject();
    QPoint oldPos(oldPointObj["x"].toInt(), oldPointObj["y"].toInt());
    QJsonObject newPointObj = obj["oldPosition"].toObject();
    QPoint newPos(newPointObj["x"].toInt(), newPointObj["y"].toInt());

    // transforms
    QJsonObject oldTransformObj = obj["oldTransform"].toObject();
    QTransform oldTransform(
      oldTransformObj["m11"].toDouble(), oldTransformObj["m12"].toDouble(), oldTransformObj["m13"].toDouble(),
      oldTransformObj["m21"].toDouble(), oldTransformObj["m22"].toDouble(), oldTransformObj["m23"].toDouble(),
      oldTransformObj["m31"].toDouble(), oldTransformObj["m32"].toDouble(), oldTransformObj["m33"].toDouble()
    );
    QJsonObject newTransformObj = obj["newTransform"].toObject();
    QTransform newTransform(
      newTransformObj["m11"].toDouble(), newTransformObj["m12"].toDouble(), newTransformObj["m13"].toDouble(),
      newTransformObj["m21"].toDouble(), newTransformObj["m22"].toDouble(), newTransformObj["m23"].toDouble(),
      newTransformObj["m31"].toDouble(), newTransformObj["m32"].toDouble(), newTransformObj["m33"].toDouble()
    );
    
    // >>>
    return new TransformLayerCommand(
        layer,
        oldPos,
        newPos,
        oldTransform,
        newTransform,
        name
    );
}