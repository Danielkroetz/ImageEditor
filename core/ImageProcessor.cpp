#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ImageProcessor.h"
#include "ImageLoader.h"

#include "../layer/LayerItem.h"
#include "../undo/AbstractCommand.h"
#include "../undo/PaintStrokeCommand.h"
#include "../undo/TransformLayerCommand.h"
#include "../undo/LassoCutCommand.h"
#include "../undo/MirrorLayerCommand.h"
#include "../undo/MoveLayerCommand.h"
#include "../undo/CageWarpCommand.h"

#include <iostream>

// ----------------------- Constructor -----------------------
ImageProcessor::ImageProcessor( const QImage& image ) : m_image(image) 
{
  std::cout << "ImageProcessor::ImageProcessor(): Processing..." << std::endl;
  { 
   m_skipMainImage = true;
   m_undoStack = new QUndoStack();
   buildMainImageLayer();
  }
}

ImageProcessor::ImageProcessor()
{
  m_undoStack = new QUndoStack();
}

// ----------------------- Methods -----------------------
void ImageProcessor::buildMainImageLayer() {
  if ( !m_image.isNull() ) {
     LayerItem* newLayer = new LayerItem(m_image);
     newLayer->setIndex(0);
     newLayer->setParent(nullptr);
     newLayer->setUndoStack(m_undoStack);
     m_layers << newLayer;
   }
}

bool ImageProcessor::process( const QString& filePath ) 
{
 std::cout << "ImageProcessor::load(): filePath='" << filePath.toStdString() << "'" << std::endl;
 { 
    QFile f(filePath);
    if ( !f.open(QIODevice::ReadOnly) ) {
     qDebug() << "ImageProcessor::load(): Cannot open '" << filePath << "'!";
     return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if ( !doc.isObject() ) return false;
    QJsonObject root = doc.object();
    
    // layers
    QJsonArray layerArray = root["layers"].toArray();
    if ( !m_skipMainImage ) {
      for ( const QJsonValue& v : layerArray ) {
        QJsonObject layerObj = v.toObject();
        QString name = layerObj["name"].toString();
        int id = layerObj["id"].toInt();
        if ( id == 0 ) {
          QString filename = layerObj["filename"].toString();
          QString pathname = layerObj["pathname"].toString();
          QString fullfilename = pathname+"/"+filename;
          ImageLoader loader;
          if ( loader.load(fullfilename,true) ) {
           m_image = loader.getImage();
           buildMainImageLayer();
          } else {
            qDebug() << "ImageProcessor::load(): Cannot find '" << fullfilename << "'!";
            return false;
          }
        }
      }
    }
    int nCreatedLayers = m_layers.size();
    for ( const QJsonValue& v : layerArray ) {
      QJsonObject layerObj = v.toObject();
      int id = layerObj["id"].toInt();
      if ( id != 0 ) {
        QString name = layerObj["name"].toString();
        if ( layerObj.contains("data") ) {
         // qDebug() << "ImageProcessor::load(): Creating new layer " << name << ": id " << id << "...";
         QString imgBase64 = layerObj["data"].toString();
         QByteArray ba = QByteArray::fromBase64(imgBase64.toUtf8());
         QImage image;
         image.loadFromData(ba,"PNG");
         // >>>
         LayerItem* newLayer = new LayerItem(image);
         newLayer->setName(name);
         newLayer->setIndex(id);
         newLayer->setParent(nullptr);
         newLayer->setUndoStack(m_undoStack);
         m_layers << newLayer;
         nCreatedLayers += 1;
        }
      }
    }
  
    // --- Restore Undo/Redo Stack ---
    QJsonArray undoArray = root["undoStack"].toArray();
    for ( const QJsonValue& v : undoArray ) {
        QJsonObject cmdObj = v.toObject();
        QString type = cmdObj["type"].toString();
        QString text = cmdObj["text"].toString();
        qDebug() << "ImageProcessor::load(): Found undo call: type=" << type << ", text=" << text;
        AbstractCommand* cmd = nullptr;
        if ( type == "PaintStrokeCommand" ) {
           cmd = PaintStrokeCommand::fromJson(cmdObj, m_layers);
        } else if ( type == "LassoCutCommand" ) {
           cmd = LassoCutCommand::fromJson(cmdObj, m_layers);
        } else if ( type == "MoveLayer" ) {
           cmd = MoveLayerCommand::fromJson(cmdObj, m_layers);
        } else if ( type == "MirrorLayer" ) {
           cmd = MirrorLayerCommand::fromJson(cmdObj, m_layers);
        } else if ( type == "CageWarp" ) {
           cmd = CageWarpCommand::fromJson(cmdObj, m_layers);
        } else if ( type == "TransformLayerCommand" ) {
           cmd = TransformLayerCommand::fromJson(cmdObj, m_layers);
        } else {
           qDebug() << "ImageProcessor::load(): Not yet processed.";
        }
        // ggf. weitere Command-Typen hier hinzufügen
        if ( cmd )
            m_undoStack->push(cmd);
    }
    
    // ---  combine layer images ---
    if ( setOutputImage(0) ) {
      for ( auto* item : m_layers ) {
       auto* layer = dynamic_cast<LayerItem*>(item);
       if ( layer && layer->id() != 0 ) {
         QImage overlayImage = layer->image();
         if ( !overlayImage.isNull() ) {
          int x = layer->pos().x();
          int y = layer->pos().y();
          QPainter painter(&m_outImage);
           painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
           painter.drawImage(x,y,overlayImage);
          painter.end();
         }
       }
      }
    } else {
      qInfo() << "Warning: Malfunction in ImageProcessor::setOutputImage().";
      return false;
    }
    
    return true;
 }
}

// ----------------------- Main image -----------------------
bool ImageProcessor::setOutputImage( int ident )
{
  std::cout << "ImageProcessor::setOutputImage(): ident=" << ident << std::endl;
  {
     for ( auto* item : m_layers ) {
       auto* layer = dynamic_cast<LayerItem*>(item);
       if ( layer && layer->id() == ident ) {
         m_outImage = layer->image(ident); 
         return true;
       }
     }
     return false;
  }
}

// ----------------------- Misc -----------------------
void ImageProcessor::printself()
{
  std::cout << "ImageProcessor::printself(): Processing..." << std::endl;
  {
    for ( auto* item : m_layers ) {
     auto* layer = dynamic_cast<LayerItem*>(item);
     if ( layer ) {
       layer->printself();
     }
    }
  }
} 
