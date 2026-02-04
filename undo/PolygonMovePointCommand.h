#pragma once

#include "AbstractCommand.h"
#include "../layer/EditablePolygon.h"

class LayerItem;

class PolygonMovePointCommand : public AbstractCommand
{

public:
    PolygonMovePointCommand( EditablePolygon* poly, int idx,
                            const QPointF& oldPos, const QPointF& newPos,
                            QUndoCommand* parent = nullptr );

    QString type() const override { return "MovePolygonPoint"; }
    LayerItem* layer() const override { return nullptr; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonMovePointCommand* fromJson( const QJsonObject& obj, EditablePolygon* poly );

private:

    EditablePolygon* m_poly;
    int m_idx;
    QPointF m_oldPos;
    QPointF m_newPos;

};
