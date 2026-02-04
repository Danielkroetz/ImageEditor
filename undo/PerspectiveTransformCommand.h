#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QVector>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"

// -----------------  ----------------- 
class PerspectiveTransformCommand : public AbstractCommand
{

public:

    PerspectiveTransformCommand( LayerItem* layer,
                                const QVector<QPointF>& before,
                                const QVector<QPointF>& after );

    LayerItem* layer() const override { return m_layer; }
    QString type() const override { return "PerspectiveTransform"; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PerspectiveTransformCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers );

private:

    LayerItem* m_layer;
    QVector<QPointF> m_before;
    QVector<QPointF> m_after;
};