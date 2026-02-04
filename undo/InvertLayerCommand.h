#pragma once

#include <QUndoCommand>
#include <QImage>
#include <QVector>
#include <QRgb>

#include "AbstractCommand.h"

class LayerItem;

class InvertLayerCommand : public AbstractCommand
{
public:
    explicit InvertLayerCommand( LayerItem* layer, const QVector<QRgb>& lut, int idx = -1,
                                     QUndoCommand* parent = nullptr );

    QString type() const override { return "InvertLayer"; }
    
    void undo() override;
    void redo() override;
    
    LayerItem* layer() const override { return m_layer; }
    
    QJsonObject toJson() const override;
    static InvertLayerCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers );

private:
    int m_layerId;
    LayerItem* m_layer;
    QImage     m_backup;
    QVector<QRgb> m_lut;
};