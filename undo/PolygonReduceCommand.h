#pragma once

#include "AbstractCommand.h"
#include "../layer/EditablePolygon.h"

class LayerItem;

class PolygonReduceCommand : public AbstractCommand
{

public:
    PolygonReduceCommand( EditablePolygon* poly,
                            QUndoCommand* parent = nullptr );

    QString type() const override { return "ReducePolygon"; }
    
    LayerItem* layer() const override { return nullptr; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonReduceCommand* fromJson( const QJsonObject& obj, EditablePolygon* poly );

private:

    EditablePolygon* m_poly;
    
    QPolygonF m_before;
    
};