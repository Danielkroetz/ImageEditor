#pragma once

#include <QDebug>
#include <QGraphicsScene>
#include <QUndoCommand>
#include <QImage>
#include <QPainter>
#include <iostream>

#include "AbstractCommand.h"
#include "../layer/LayerItem.h"
#include "../layer/Layer.h"

class LassoCutCommand : public AbstractCommand
{

public:
    LassoCutCommand( LayerItem* originalLayer, LayerItem* newLayer, const QRect& bounds, 
                         const QImage& originalBackup, const int index, const QString& name, QUndoCommand* parent=nullptr );
    
    QString type() const override { return "LassoCut"; }
    
    void undo() override;
    void redo() override;
    
    LayerItem* layer() const override { return m_newLayer; }
    
    QJsonObject toJson() const override;
    static LassoCutCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent = nullptr );
    
    int layerId() const { return m_newLayerId; }
    void setController( QUndoCommand *undoCommand ) { m_controller = undoCommand; };
    void save_backup() {
      m_backup.save("/tmp/imageeditor_backuppic.png");
    }

private:

    int m_originalLayerId = -1;
    int m_newLayerId = -1;
    
    QUndoCommand* m_controller = nullptr;
    LayerItem* m_originalLayer = nullptr;
    LayerItem* m_newLayer = nullptr;
    
    QString m_name;
    QRect m_bounds;
    QImage m_backup;
    
};
