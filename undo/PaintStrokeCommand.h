#pragma once

#include <QUndoCommand>
#include <QColor>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"

class PaintStrokeCommand : public AbstractCommand
{

public:

    PaintStrokeCommand( LayerItem* layer, const QPoint& pos, const QColor& color, int radius, qreal hardness, QUndoCommand* parent = nullptr );
    PaintStrokeCommand( LayerItem* layer, const QVector<QPoint>& strokePoints, const QColor& color, int radius, float hardness, QUndoCommand* parent = nullptr );

    QString type() const override { return "PaintStroke"; }
    
    void undo() override;
    void redo() override;
    
    QJsonObject toJson() const override;
    static PaintStrokeCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent = nullptr );
    static PaintStrokeCommand* fromJson( const QJsonObject& obj, LayerItem* layer );
    
private:

	void paint( QImage& img );

private:

    int m_layerId;
    LayerItem* m_layer = nullptr;
    
    QVector<QPoint>   m_points;
    QPoint     m_pos;
    int        m_radius = 1;
    qreal      m_hardness = 1.0;
    QColor     m_color;
    
    QRect      m_dirtyRect;
    QRect      m_backupRect;
    QImage     m_backup;
    
};