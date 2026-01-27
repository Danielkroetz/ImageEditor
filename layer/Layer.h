#pragma once

#include <QGraphicsItem>
#include <QImage>

class Layer {

  public:
  
    Layer( int index = -1, const QImage& image = QImage() ) : m_id(index), m_image(image) {}
  
    int id() const { return m_id; }
    float opacity() const { return m_opacity; }
    QString name() const { return m_name; }

  public:
  
    int m_id = -1;
    float m_opacity = 1.0;
    QString m_name;
    
    bool m_visible = true;
    QImage m_image;
    QPolygon m_polygon;
    QGraphicsItem* m_item = nullptr;
    bool m_linkedToImage = false;
    
    Layer() = default;
};