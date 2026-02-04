#pragma once

#include <QUndoCommand>
#include <QTransform>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"

class TransformLayerCommand : public AbstractCommand
{

public:

    TransformLayerCommand(
        LayerItem* layer,
        const QPointF& oldPos, 
        const QPointF& newPos,
        const QTransform& oldTransform,
        const QTransform& newTransform,
        const QString &name="Transform Layer",
        QUndoCommand* parent = nullptr
    );
    
    QString type() const override { return "TransformLayer"; }
    LayerItem* layer() const override { return m_layer; }

    void undo() override;
    void redo() override;
    
    QJsonObject toJson() const override;
    static TransformLayerCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent = nullptr );

private:

    int m_layerId = -1;
    LayerItem* m_layer = nullptr;
    
    QString m_name;
    QPointF m_oldPos;
    QPointF m_newPos;
    QTransform m_oldTransform;
    QTransform m_newTransform;
    
};