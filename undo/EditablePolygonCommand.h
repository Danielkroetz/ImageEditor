#pragma once

#include "AbstractCommand.h"

#include "../layer/LayerItem.h"
#include "../layer/EditablePolygon.h"
#include "../layer/EditablePolygonItem.h"

#include <QGraphicsScene>

class EditablePolygonCommand : public AbstractCommand
{

public:

    EditablePolygonCommand( LayerItem* layer, QGraphicsScene* scene, const QPolygonF& polygon, const QString& name, QUndoCommand* parent = nullptr );

    QString type() const override { return "EditablePolygon"; }
    EditablePolygon* model() const { return m_model; }
    
    void setVisible( bool isVisible );
    
    LayerItem* layer() const override { return m_layer; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static EditablePolygonCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QGraphicsScene* scene = nullptr );

private:

    LayerItem* m_layer = nullptr;
    QGraphicsScene* m_scene = nullptr;

    // Daten
    QString m_name;
    QPolygonF m_polygon;

    // Runtime-Objekte
    EditablePolygon* m_model = nullptr;
    EditablePolygonItem* m_item = nullptr;
    
};
