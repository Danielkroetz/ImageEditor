#pragma once

#include <QUndoCommand>
#include <QPointF>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"

class MirrorLayerCommand : public AbstractCommand
{

public:
    MirrorLayerCommand( LayerItem* layer, const int idx, int mirrorPlane, QUndoCommand* parent = nullptr )
        : AbstractCommand(parent), m_layer(layer), m_mirrorPlane(mirrorPlane), m_layerId(idx)
    {
        QString planeName = mirrorPlane == 1 ? "Vertical" : "Horizontal";
        setText(QString("Mirror %1 Layer %2").arg(planeName).arg(idx));
    }
    
    QString type() const override { return "MirrorLayer"; }
    void undo() override { if ( m_layer ) m_layer->setMirror(m_mirrorPlane); }
    void redo() override { if ( m_layer ) m_layer->setMirror(m_mirrorPlane); }
    
    QJsonObject toJson() const override;
    static MirrorLayerCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers );

private:
    int m_layerId;
    int m_mirrorPlane;
    LayerItem* m_layer;
    
};