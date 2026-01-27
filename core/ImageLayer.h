#pragma once

#include <QImage>

// -------------------------- Image Layer --------------------------
class ImageLayer {

public:

	ImageLayer( const QImage& image ) : m_image(image) {
	}
	
	int id() const { return m_index; }
	void setIndex( const int index ) { m_index = index; }
	
protected:

	QImage m_image;
	QImage m_originalImage;
	
	
private:

	int m_index = 0;

};