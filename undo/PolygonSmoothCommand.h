#pragma once

#include "AbstractCommand.h"
#include "../layer/EditablePolygon.h"

class LayerItem;

class PolygonSmoothCommand : public AbstractCommand
{

public:
    PolygonSmoothCommand( EditablePolygon* poly,
                            QUndoCommand* parent = nullptr );

    QString type() const override { return "SmoothPolygon"; }
    
    LayerItem* layer() const override { return nullptr; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonSmoothCommand* fromJson( const QJsonObject& obj, EditablePolygon* poly );

private:

    EditablePolygon* m_poly;
    
    QPolygonF m_before;
    
};