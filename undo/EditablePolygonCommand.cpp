#include "EditablePolygonCommand.h"
#include "../undo/PolygonMovePointCommand.h"
#include <iostream>

// ------------------------ Constructor ------------------------
EditablePolygonCommand::EditablePolygonCommand( LayerItem *layer, QGraphicsScene* scene,
                                           const QPolygonF& polygon, const QString& name, QUndoCommand* parent )
    : AbstractCommand(parent)
    , m_scene(scene)
    , m_layer(layer)
    , m_polygon(polygon)
    , m_name(name)
{
  std::cout << "EditablePolygonCommand::EditablePolygonCommand(): name=" << name.toStdString() << std::endl;
  setText(QString("Editable %1").arg(name));
  m_model = new EditablePolygon(m_name);
  m_model->setPolygon(m_polygon);
  m_item = new EditablePolygonItem(m_model,m_layer);
}

// ------------------------ Methods ------------------------
void EditablePolygonCommand::redo()
{
   qDebug() << "EditablePolygonCommand::redo(): Processing...";
   {
    if ( !m_model ) {
        m_model = new EditablePolygon(m_name);
        m_model->setPolygon(m_polygon);
        m_item = new EditablePolygonItem(m_model,m_layer);
    }
    m_model->setVisible(true);
    if ( !m_item->scene() )
        m_scene->addItem(m_item);
   }
}

void EditablePolygonCommand::undo()
{
  qDebug() << "EditablePolygonCommand::undo(): Processing...";
  {
    if ( m_item && m_item->scene() )
        m_scene->removeItem(m_item);
  }
}

// ------------------------ Methods ------------------------
QJsonObject EditablePolygonCommand::toJson() const
{
    QJsonObject obj = AbstractCommand::toJson();
    obj["type"] = "EditablePolygonCommand";
    obj["name"] = m_model != nullptr ? m_model->name() : "Unknown";
    obj["layerId"] = m_layer != nullptr ? m_layer->id() : 0;
    QJsonArray pts;
    for ( const QPointF& p : m_polygon ) {
        QJsonObject jp;
        jp["x"] = p.x();
        jp["y"] = p.y();
        pts.append(jp);
    }
    obj["points"] = pts;
    obj["undo"] = m_model->undoStackToJson();
    return obj;
}

EditablePolygonCommand* EditablePolygonCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers,
                                                     QGraphicsScene* scene )
{
  std::cout << "EditablePolygonCommand::fromJson(): Processing..." << std::endl;
  {
    QString name = obj.value("name").toString("Unknown");
    const int layerId = obj["layerId"].toInt(-1);
    LayerItem* layer = AbstractCommand::getLayerItem(layers,layerId);
    if ( !layer ) {
      qWarning() << "EditablePolygonCommand::fromJson(): Layer " << layerId << " not found.";
      return nullptr;
    }
    QPolygonF poly;
    QJsonArray pts = obj["points"].toArray();
    for ( const QJsonValue& v : pts ) {
        QJsonObject jp = v.toObject();
        poly << QPointF(jp["x"].toDouble(), jp["y"].toDouble());
    }
    EditablePolygonCommand* editablePolygonCommand = new EditablePolygonCommand( layer, layer->scene(), poly, name );
    EditablePolygon *model = editablePolygonCommand->model();
    if ( model != nullptr ) {
      model->undoStackFromJson(obj["undo"].toArray());
    }
    return editablePolygonCommand;
  }
}
