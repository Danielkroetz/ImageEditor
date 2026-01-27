#pragma once

#include <QUndoCommand>
#include <QPointF>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"

class MoveLayerCommand : public AbstractCommand
{

public:
    MoveLayerCommand( LayerItem* layer, const QPointF& oldPos, const QPointF& newPos, const int idx=0, QUndoCommand* parent = nullptr )
        : AbstractCommand(parent), m_layer(layer), m_oldPos(oldPos), m_newPos(newPos), m_layerId(idx)
    {
        setText(QString("Move Layer %1").arg(idx));
    }
    
    QString type() const override { return "MoveLayer"; }
    void undo() override { if ( m_layer ) m_layer->setPos(m_oldPos); }
    void redo() override { if (m_layer) m_layer->setPos(m_newPos); }
    
    QJsonObject toJson() const override;
    static MoveLayerCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers );

private:
    int m_layerId;
    LayerItem* m_layer;
    QPointF m_oldPos;
    QPointF m_newPos;
    
};