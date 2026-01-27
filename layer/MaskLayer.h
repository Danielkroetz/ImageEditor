#pragma once

#include <QObject>
#include <QImage>
#include <QVector>
#include <QSize>

class MaskLayer : public QObject {

	Q_OBJECT

public:
    explicit MaskLayer( const QSize& size );
    explicit MaskLayer( int width, int height );
    
    void setPixel( int x, int y, uchar label );
    uchar pixel( int x, int y ) const;
    
    int width() { return m_image.width(); }
    int height() { return m_image.height(); }
    const QImage& image() const { return m_image; }
    void clear();
    
    quint8 labelAt( int x, int y ) const;
    void setLabelAt( int x, int y, quint8 label );
    void setImage( const QImage& image );

    const QVector<quint8>& data() const;
    
    void emitChanged() { emit changed(); }
    
signals:
	void changed();

private:
    QImage m_image;
    int m_width;
    int m_height;
    QVector<quint8> m_labels; // 0..9
    
};