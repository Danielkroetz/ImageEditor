#include "LassoCutCommand.h"

#include <QByteArray>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <iostream>

// ---------------------- Constructor ----------------------
LassoCutCommand::LassoCutCommand( LayerItem* originalLayer, LayerItem* newLayer, const QRect& bounds,
                    const QImage& originalBackup, const int index, QUndoCommand* parent )
        : AbstractCommand(parent),
          m_originalLayer(originalLayer),
          m_newLayer(newLayer),
          m_bounds(bounds),
          m_backup(originalBackup)
{
  std::cout << "LassoCutCommand::LassoCutCommand(): Processing..." << std::endl;
  {
    newLayer->setPos(bounds.topLeft());
    m_originalLayerId = originalLayer->id();
    m_newLayerId = newLayer->id(); 
    setText(QString("Lasso %1 Cut").arg(index));
  }
}

// ---------------------- Undo/Redo ----------------------
void LassoCutCommand::undo()
{
  std::cout << "LassoCutCommand::redo(): Processing..." << std::endl;
  {
    QImage tempImage = m_originalLayer->image();
    QPainter p(&tempImage);
     for ( int y = 0; y < m_backup.height(); ++y ) {
      for ( int x = 0; x < m_backup.width(); ++x ) {
        QColor maskPixel = m_backup.pixelColor(x, y);
        if ( maskPixel.alpha() > 0 ) {
          tempImage.setPixelColor(m_bounds.x()+x,m_bounds.y()+y,maskPixel);
        }
      }
     }
    p.end();
    m_originalLayer->setImage(tempImage);
    m_originalLayer->update();
    if ( m_newLayer->scene() ) {
      m_newLayer->scene()->removeItem(m_newLayer);
    }     
    m_newLayer->setVisible(false);
  }
}
    
void LassoCutCommand::redo()
{
  std::cout << "LassoCutCommand::redo(): Processing..." << std::endl;
  {
    QColor color = QColor(255,255,255);
    QImage tempImage = m_originalLayer->image();
    for ( int y = 0; y < m_backup.height(); ++y ) {
      for ( int x = 0; x < m_backup.width(); ++x ) {
        QColor maskPixel = m_backup.pixelColor(x, y);
        if ( maskPixel.alpha() > 0 ) {
          tempImage.setPixelColor(m_bounds.x()+x,m_bounds.y()+y,color);
        }
      }
    }
    m_originalLayer->setImage(tempImage);
    if ( !m_newLayer->scene() && m_originalLayer->scene() ) {
      m_originalLayer->scene()->addItem(m_newLayer);
    } else {
     /*
     std::cerr << "LassoCutCommand::redo(): No valid scene data available." << std::endl;
     std::cerr << " newLayerScene=" << ( m_newLayer->scene() ? 1 : 0 ) << std::endl;
     std::cerr << " originalLayerScene=" << ( m_originalLayer->scene() ? 1 : 0 ) << std::endl;
     */
    }
    m_newLayer->setVisible(true);
    m_originalLayer->update();
  }
}

// ---------------------- JSON ----------------------
QJsonObject LassoCutCommand::toJson() const
{
   QJsonObject obj = AbstractCommand::toJson();
   obj["originalLayerId"] = m_originalLayerId;
   obj["newLayerId"] = m_newLayerId;
   QJsonObject rectObj;
   rectObj.insert("x", m_bounds.x());
   rectObj.insert("y", m_bounds.y());
   rectObj.insert("width", m_bounds.width());
   rectObj.insert("height", m_bounds.height());
   obj["rect"] = rectObj;
   obj["type"] = "LassoCutCommand";
   return obj;
}

LassoCutCommand* LassoCutCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers, QUndoCommand* parent )
{
  std::cout << "LassoCutCommand::fromJson(): Processing..." << std::endl;
  {
    // Original Layer
    const int originalLayerId = obj["originalLayerId"].toInt(-1);
    LayerItem* originalLayer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == originalLayerId ) {
            originalLayer = l;
            break;
        }
    }
    if ( !originalLayer ) {
      qWarning() << "LassoCutCommand::fromJson(): Original layer " << originalLayerId << " not found.";
      return nullptr;
    }
    // New Layer
    const int newLayerId = obj["newLayerId"].toInt(-1);
    LayerItem* newLayer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == newLayerId ) {
            newLayer = l;
            break;
        }
    }
    if ( !newLayer ) {
      qWarning() << "LassoCutCommand::fromJson(): New layer " << newLayerId << " not found.";
      return nullptr;
    }
    // >>>
    QJsonObject r = obj["rect"].toObject();
    QRect rect(r["x"].toInt(),r["y"].toInt(),r["width"].toInt(),r["height"].toInt());
    // >>>
    return new LassoCutCommand(originalLayer,newLayer,rect,newLayer->originalImage(),newLayerId);
  }
}