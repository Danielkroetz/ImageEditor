#pragma once

#include <QUndoCommand>
#include <QVector>
#include <QPointF>
#include <QRectF>

#include "AbstractCommand.h"

class LayerItem;

class CageWarpCommand : public AbstractCommand
{

public:

    CageWarpCommand( LayerItem* layer, const QVector<QPointF>& before,
                    const QVector<QPointF>& after, const QRectF& rect, int rows, int columns, QUndoCommand* parent = nullptr );

    QString type() const override { return "LassoCut"; }
    
    void undo() override;
    void redo() override;
    
    LayerItem* layer() const override { return m_layer; }
    
    QJsonObject toJson() const override;
    static CageWarpCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent = nullptr );
    
    void pushNewWarpStep( const QVector<QPointF>& points );

private:

    int m_layerId = -1;
    
    int m_steps = 0;
    int m_rows = 0;
    int m_columns = 0;
    QString m_interpolation = "trlinear";
    QRectF m_rect;
    
    LayerItem* m_layer;
    
    QVector<QPointF> m_before;  // Startposition der Cage-Punkte
    QVector<QPointF> m_after;   // Endposition der Cage-Punkte
    
};