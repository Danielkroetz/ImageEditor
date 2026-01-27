#pragma once

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QUndoStack>

// --- ---
class LayerItem;

// -------------------------- ImageProcessor --------------------------
class ImageProcessor {

public:

    ImageProcessor();
    ImageProcessor( const QImage& image );
    
    QImage getOutputImage() const { return m_outImage; }
    
    // --------------------------  --------------------------
    bool setOutputImage( int ident );
    bool process( const QString& filePath );
    void printself();
   
    
private:

    bool m_skipMainImage = false;
   
    QImage m_image;
    QImage m_outImage;
    
    QUndoStack* m_undoStack = nullptr;
    
    QList<LayerItem*> m_layers;
    
    void buildMainImageLayer();

};