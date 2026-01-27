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
                         const QImage& originalBackup, const int index, QUndoCommand* parent=nullptr );
    
    QString type() const override { return "LassoCut"; }
    
    void undo() override;
    void redo() override;
    
    QJsonObject toJson() const override;
    static LassoCutCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent = nullptr );
    
    void redo123() {
        QImage tempImage = m_originalLayer->image();
        QPainter p(&tempImage);
         p.setCompositionMode(QPainter::CompositionMode_Clear); // Macht den Bereich transparent
         p.fillRect(m_bounds, Qt::transparent);
        p.end();
        m_originalLayer->setImage(tempImage);
        if (m_newLayer->scene() == nullptr && m_originalLayer->scene()) {
         m_originalLayer->scene()->addItem(m_newLayer);
        }
        m_newLayer->setVisible(true);
        m_originalLayer->update();
    }
    
    void save_backup() {
      m_backup.save("/tmp/imageeditor_backuppic.png");
    }

private:

    int m_originalLayerId = -1;
    int m_newLayerId = -1;
    
    LayerItem* m_originalLayer;
    LayerItem* m_newLayer;
    QRect m_bounds;
    QImage m_backup;
    
};
