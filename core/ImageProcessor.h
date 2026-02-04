#pragma once

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QUndoStack>

// --- ---
class AbstractCommand;
class LayerItem;

// -------------------------- ImageProcessor --------------------------
class ImageProcessor {

public:

    ImageProcessor();
    ImageProcessor( const QImage& image );
    
    QImage getOutputImage() const { return m_outImage; }
    
    // --------------------------  --------------------------
    void setIntermediatePath( const QString& path = "", const QString& outname = "" );
    bool setOutputImage( int ident );
    bool process( const QString& filePath );
    void printself();
   
    
private:

    QString saveIntermediate( AbstractCommand *cmd, const QString &name, int step );

    bool m_skipMainImage = false;
    bool m_saveIntermediate = false;
   
    QImage m_image;
    QImage m_outImage;
    
    QString m_intermediatePath = "";
    QString m_basename = "";
    
    QUndoStack* m_undoStack = nullptr;
    
    QList<LayerItem*> m_layers;
    
    void buildMainImageLayer();

};