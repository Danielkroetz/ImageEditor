#pragma once

#include <QObject>
#include <QImage>
#include <QPixmap>

// -------------------------- ImageLoader --------------------------
class ImageLoader {

public:

    QPixmap getPixmap() const { return m_pixmap; }
    QImage getImage() const { return m_image; }

    bool load( const QString& filePath, bool asImage=false );
    bool saveAs( const QImage& image, const QString& fileName );
    
private:

	QPixmap loadMincAsPixmap( const QString& ); 
	QImage loadMincImage( const QString& );
	
	QPixmap m_pixmap;
	QImage m_image;

};