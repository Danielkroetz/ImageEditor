#pragma once

#include "AbstractCommand.h"
#include "../layer/EditablePolygon.h"

class LayerItem;

class PolygonDeletePointCommand : public AbstractCommand
{

public:

    PolygonDeletePointCommand( EditablePolygon* poly,
                              int idx,
                              const QPointF& p,
                              QUndoCommand* parent = nullptr );

    QString type() const override { return "DeletePolygonPoint"; }
    LayerItem* layer() const override { return nullptr; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonDeletePointCommand* fromJson( const QJsonObject& obj, EditablePolygon* poly );

private:

    EditablePolygon* m_poly;
    int m_idx;
    QPointF m_point;
    
};