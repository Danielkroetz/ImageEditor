/*
 * Layer class
 *
 */
 
#include "LayerItem.h"
#include "Layer.h"
#include "TransformHandleItem.h"
#include "CageControlPointItem.h"
#include "CageOverlayItem.h"

#include "../gui/MainWindow.h"
#include "../gui/ImageView.h"
#include "../undo/TransformLayerCommand.h"
#include "../undo/MirrorLayerCommand.h"
#include "../undo/MoveLayerCommand.h"
#include "../util/Interpolation.h"
#include "../util/GeometryUtils.h"
#include "../util/TriangleWarp.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QCryptographicHash>
#include <QByteArray>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QPainter>
#include <iostream>

// ------------------------ LayerItem ------------------------
LayerItem::LayerItem( const QPixmap& pixmap, QGraphicsItem* parent ) : QGraphicsPixmapItem(pixmap,parent)
{ 
  // std::cout << "LayerItem::LayerItem(): Pixmap processing..." << std::endl;
  {
    m_image = pixmap.toImage();
    m_originalImage = pixmap.toImage();
    init();
  }
}

LayerItem::LayerItem( const QImage& image, QGraphicsItem* parent ) 
      : QGraphicsPixmapItem(parent)
      , m_originalImage(image)
      , m_image(image)
{
   if ( qobject_cast<QApplication*>(qApp) ) {
    setPixmap(QPixmap::fromImage(m_image));
   } else {
    m_nogui = true;
   }
   init();
}

// per default moveable layer item
// NOTE: QGraphicsItem::ItemIsSelectable expands the size of the QGraphicsPixmapItem by +1+1 !   
void LayerItem::init() 
{
  setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemIsFocusable);
  setTransformationMode(Qt::SmoothTransformation);
  setAcceptedMouseButtons(Qt::LeftButton);
  setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
  
  // setZValue(0);
  m_showBoundingBox = true;

  // default color  
  m_lassoPen = QPen(Qt::green);
  m_lassoPen.setWidth(0); // immer 1px auf Bildschirm
  m_lassoPen.setStyle(Qt::SolidLine);
  
  // Selektions-Pen
  m_selectedPen = QPen(Qt::red);
  m_selectedPen.setWidth(0);
  m_selectedPen.setStyle(Qt::SolidLine);
  
}

// ------------------------ Self info ------------------------
void LayerItem::printself()
{
  std::cout << " LayerItem::printself(): name=" << name().toStdString() << ", id=" << m_index << ", visible=" << isVisible() << std::endl;
  QRectF rect = m_nogui ? m_image.rect() : boundingRect();
  QString geometry = QString("%1x%2+%3+%4").arg(rect.width()).arg(rect.height()).arg(pos().x()).arg(pos().y());
  std::cout << "  + geometry=" << geometry.toStdString() << std::endl;
  std::cout << "  + cage: enabled=" << m_cageEnabled << ", edited=" << m_cageEditing << std::endl;
  std::cout << "  + bounding box=" << m_showBoundingBox << std::endl;
}

// ------------------------ Getter / Setter ------------------------
QImage& LayerItem::image( int id ) {
    return id == 1 ? m_originalImage : m_image;
}
void LayerItem::setImage( const QImage &image ) {
    m_image = image;
    updatePixmap();
}
void LayerItem::setLayer( Layer *layer ) {
    m_layer = layer;
}

void LayerItem::setType( LayerType layerType ) {
    m_type = layerType;
    if ( m_type == LayerType::MainImage ) {
      setFlag(QGraphicsItem::ItemIsMovable, false);
      setAcceptedMouseButtons(Qt::NoButton);
    } else {
      setFlag(QGraphicsItem::ItemIsMovable, true);
      setAcceptedMouseButtons(Qt::LeftButton);
      setAcceptedMouseButtons(Qt::RightButton);
    }
}

void LayerItem::setFileInfo( const QString &filePath )
{
  // set filename
  m_filename = filePath;
  // compute checksum
  QFile file(filePath);
  if ( file.open(QIODevice::ReadOnly) ) {
   QCryptographicHash hash(QCryptographicHash::Md5);
   while ( !file.atEnd() ) {
     hash.addData(file.read(8192));
   }
   m_checksum = hash.result().toHex();
  }
}

const QImage& LayerItem::originalImage() {
	if ( m_originalImage.format() != QImage::Format_ARGB32 )
        m_originalImage = m_originalImage.convertToFormat(QImage::Format_ARGB32);
    return m_originalImage;
}

QString LayerItem::name() const {
  // if ( m_name != "" ) return m_name;
  if ( !m_layer ) return "MainImage";
  return m_layer->name();
}

// ------------------------ Update ------------------------
void LayerItem::updatePixmap() {
  if ( qobject_cast<QApplication*>(qApp) ) {
    setPixmap(QPixmap::fromImage(m_image));
  }
}

void LayerItem::updateImageRegion( const QRect& rect ) {
  if ( rect.isEmpty() )
    return;
  QPixmap pm = pixmap();
  if ( pm.isNull() )
    return;
  QPainter painter(&pm);
   painter.setCompositionMode(QPainter::CompositionMode_Source);
   painter.drawImage(rect.topLeft(),m_image,rect);
  painter.end();
  setPixmap(pm);
  update(rect);
}

void LayerItem::updateOriginalImage() {
  m_originalImage = m_image;
}

// ------------------------ Mirror ------------------------
void LayerItem::setMirror( int mirrorPlane ) {
  // std::cout << "LayerItem::setMirror(): mirrorPlane=" << mirrorPlane << std::endl;
  {
    #if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
     QImage flippedImage = m_image.flipped(mirrorPlane == 1 ? Qt::Vertical : Qt::Horizontal);
    #else
     QImage flippedImage = m_image.mirrored(mirrorPlane == 1 ? Qt::Vertical : Qt::Horizontal);
    #endif
    m_image = flippedImage;
    updatePixmap();
  }
}

// ------------------------ Transform ------------------------
void LayerItem::setImageTransform( const QTransform& transform, bool combine ) {
  if ( qobject_cast<QApplication*>(qApp) ) {
    QGraphicsPixmapItem::setTransform(transform,combine);
    return;
  }
  // ---- not yet correct ---
  QPointF centerPoint = scenePos() + QPointF(m_image.width() / 2.0, m_image.height() / 2.0);
  QTransform finalTrafo;
  finalTrafo.translate(m_image.width() / 2.0, m_image.height() / 2.0);
  finalTrafo *= transform;
  finalTrafo.translate(-m_image.width() / 2.0, -m_image.height() / 2.0);
  m_image = m_image.transformed(finalTrafo, Qt::SmoothTransformation);
  qreal newCenterX = m_image.width() / 2.0;
  qreal newCenterY = m_image.height() / 2.0;
  setPos(centerPoint.x() - newCenterX, centerPoint.y() - newCenterY);
}

// ------------------------ Paint ------------------------
void LayerItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
  // std::cout << "LayerItem::paint(): Processing " << (m_layer?m_layer->name.toStdString():"unknown") << " ..." << std::endl;
  QGraphicsPixmapItem::paint(painter,option,widget);
  if ( isSelected() ) {
    painter->setPen(m_selectedPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
  } else if ( m_showBoundingBox ) {
    painter->setPen(m_lassoPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
  }
  // render cage
  if ( m_cageEnabled ) {
   painter->setPen(QPen(Qt::red,0));
   painter->setBrush(Qt::NoBrush);
   painter->drawPolygon(m_cage.data(), m_cage.size());
  }
}

void LayerItem::paintStrokeSegment( const QPoint& p0, const QPoint& p1, const QColor &color, int radius, float hardness )
{
  // std::cout << "LayerItem::paintStrokeSegment(): Processing..." << std::endl;
  if ( radius < 0.0 ) return;
  {
    const float spacing = std::max(1.0f, radius * 0.35f);
    const float dx = p1.x() - p0.x();
    const float dy = p1.y() - p0.y();
    const float dist = std::sqrt(dx*dx + dy*dy);
    if ( dist <= 0.0f ) {
     Interpolation::dab(m_image, p0, color, radius, hardness);
     return;
    }
    const int steps = std::ceil(dist / spacing);
    for ( int i = 0; i <= steps; ++i ) {
        float t = float(i) / float(steps);
        QPoint p(
            int(p0.x() + t * dx),
            int(p0.y() + t * dy)
        );
        Interpolation::dab(m_image, p, color, radius, hardness);
    }
    // ---
    QRect dirtyRect = QRect(p0, QSize(1,1));
    dirtyRect |= QRect(p1, QSize(1,1));
    int pad = radius + 2;
    dirtyRect.adjust(-pad, -pad, pad, pad);
    dirtyRect &= m_image.rect();
    updateImageRegion(dirtyRect);
    // ---
    // setPixmap(QPixmap::fromImage(m_image));
  }
}

// ------------------------ Cage ------------------------
void LayerItem::applyTriangleWarp()
{
  std::cout << "LayerItem::applyTriangleWarp(): meshActive=" << m_mesh.isActive() << ",  m_cageEnabled=" << m_cageEnabled << ", m_cageEditing=" << m_cageEditing << std::endl;
  {
    TriangleWarp::WarpResult warped = TriangleWarp::warp(m_originalImage,m_mesh);
    if ( !warped.image.isNull() ) {
     setPixmap(QPixmap::fromImage(warped.image));
     // setPos(warped.offset);
    } 
  }
}

void LayerItem::applyCageWarp()
{
  std::cout << "DEPRECTAED **** LayerItem::applyCageWarp(): Processing..." << std::endl;
}

void LayerItem::enableCage( int cols, int rows )
{
  std::cout << "LayerItem::enableCage(): cols=" << cols << ", rows=" << rows << ", enabled=" << m_cageEnabled << std::endl;
  {
   /* BETTER 
    if ( m_cageEnabled ) {
      m_mesh.update(cols,rows);
      return;
    }
`  */
    m_mesh.create(boundingRect(), cols, rows);
    m_cageEnabled = true;
    if ( !scene() )
      return;
    if ( !m_cageOverlay ) {
      m_cageOverlay = new CageOverlayItem(this);
      m_cageOverlay->setParentItem(this);
    }
    m_cageOverlay->setVisible(true);
    // Handles
    qDeleteAll(m_handles);
    m_handles.clear();
    for ( int i = 0; i < m_mesh.pointCount(); ++i ) {
      auto* h = new CageControlPointItem(this, i);
      h->setParentItem(this);
      h->setPos(m_mesh.point(i));
      m_handles << h;
    }
  }
}

// --- what about the grid ??? ---
void LayerItem::disableCage()
{
  std::cout << "LayerItem::disableCage(): m_cageEditing=" << m_cageEditing << "|" << m_cageEnabled << " nControlPoints=" << m_mesh.pointCount() << std::endl;
  {
    if ( m_cageOverlay != nullptr ) {
     if ( m_parent != nullptr ) {
      MainWindow* parent = dynamic_cast<MainWindow*>(m_parent);
      if ( parent && parent->getViewer() != nullptr ) {
       parent->getViewer()->setActiveCageLayer(nullptr);
      }
     }    
     m_mesh.setActive(false);
     m_cageEnabled = false;
     m_cageEditing = false;
     for ( int i = 0; i < m_handles.size(); ++i ) {
       m_handles[i]->setVisible(false);
     }
     m_cageOverlay->setVisible(false);
     update();
    }
  }
}

void LayerItem::setNumberOfActiveCagePoints( int nControlPoints ) {
  enableCage(nControlPoints,nControlPoints);
}

void LayerItem::setCageWarpRelaxationSteps( int nRelaxationsSteps ) {
  m_nCageWarpRelaxationsSteps = nRelaxationsSteps;
}

void LayerItem::setCagePoints( const QVector<QPointF>& pts ) {
    m_mesh.setPoints(pts);
    update();
}

QVector<QPointF> LayerItem::cagePoints() const {
    return m_mesh.points();
}

void LayerItem::updateCagePoint( TransformHandleItem* handle, const QPointF& localPos )
{
  // std::cout << "LayerItem::updateCagePoint(): Processing..." << std::endl;
  {
    int idx = m_handles.indexOf(handle);
    if ( idx < 0 ) return;
    m_cage[idx] = localPos;
    update(); // neu zeichnen
  }
}

void LayerItem::setCagePoint( int idx, const QPointF& pos )
{
  // std::cout << "LayerItem::setCagePoint(): inde=" << idx << ", pos=(" << pos.x() << ":" << pos.y() << ")..." << std::endl;
  {
    QPointF local = mapFromScene(pos);
    m_mesh.setPoint(idx, local);
    for ( int i = 0; i < m_handles.size(); ++i )
        m_handles[i]->setPos(m_mesh.point(i));
    m_mesh.relax(m_nCageWarpRelaxationsSteps);
    update();
  
  /*
    if ( idx < 0 || idx >= m_cage.size() ) return;
    m_cage[idx] = pos;
    // Transformation: Mapping vom Originalpolygon auf neues Polygon
    if ( m_cage.size() >= 4 && m_originalCage.size() >= 4 ) { // Mindestens 4 Punkte für Quad-Transform
        QTransform t;
        bool ok = QTransform::quadToQuad(
            m_originalCage.mid(0, 4),
            m_cage.mid(0, 4),
            t
        );
        if ( ok ) {
            // setTransform(t, false); for life but its not working
        }
    }
    if ( idx < m_points.size() ) {
      // m_points[idx]->setPos(pos);
    }
    update();
    // qDebug() << "Index: " << idx << ", Handle pos:" << pos << "Layer transform:" << transform();
    */
    
  } 
}

void LayerItem::commitCageTransform( const QVector<QPointF> &cage )
{
  std::cout << "LayerItem::commitCageTransform(): Processing..." << std::endl;
  {
    if ( m_cage.size() < 4 || m_originalCage.size() < 4 )
        return;
    QTransform t;
    if ( QTransform::quadToQuad(m_originalCage.mid(0, 4),cage.mid(0, 4),t) ) {
        setTransform(t, false);
    }
    // Neue Basis für weitere Transformationen
    m_cage = cage;
    m_originalCage = m_cage;
    update();
  }
}

void LayerItem::beginCageEdit()
{
  std::cout << "LayerItem::beginCageEdit(): Processing..." << std::endl;
    m_cageEditing = true;
    m_startPos = pos();
    m_startTransform = transform();
}

void LayerItem::endCageEdit( int idx, const QPointF& startPos )
{
  std::cout << "LayerItem::endCageEdit(): Processing..." << std::endl;
    QVector<QPointF> cage;
    for ( int i=0 ; i<m_cage.size() ; i++ ) {
      cage.append(QPointF(m_cage[i].x(),m_cage[i].y()));
    }
    cage[idx] = startPos;
    commitCageTransform(cage);
    m_cageEditing = false;
   // if ( undoStack() && m_startTransform != transform() )
   //   undoStack()->push(new TransformLayerCommand(this, m_startPos, pos(), m_startTransform, transform()));
}

void LayerItem::setTransformMode( TransformMode mode ) 
{
   std::cout << "LayerItem::setTransformMode(): index=" << m_index << ", mode=" << m_trafomode << "|" << mode << std::endl;
   {
     if ( m_trafomode == mode )
      return;
     if ( m_trafomode == TransformMode::CageWarp ) {
      disableCage();
     }
     m_trafomode = mode;
   }
}


// ------------------------ Mouse events ------------------------
void LayerItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
  std::cout << "LayerItem::mouseDoubleClickEvent(): index=" << m_index << ", mode=" << m_trafomode << std::endl;
  {
    if ( m_trafomode == TransformMode::CageWarp ) {
      if ( m_cageEnabled == false ) {
       if ( m_parent != nullptr ) {
        MainWindow* parent = dynamic_cast<MainWindow*>(m_parent);
        if ( parent && parent->getViewer() != nullptr ) {
         parent->getViewer()->setActiveCageLayer(this);
         int nCagePoints = parent->getNumberOfCageControlPoints();
         enableCage(nCagePoints,nCagePoints);
         return;
        }
       }
       enableCage(2,2);
      } else {
       disableCage();
      }
    } else if ( m_trafomode == TransformMode::Flip ) {
     m_undoStack->push(new MirrorLayerCommand(this, m_index, 1));
    } else if ( m_trafomode == TransformMode::Flop ) {
     m_undoStack->push(new MirrorLayerCommand(this, m_index, 2));
    } else if ( m_trafomode == TransformMode::Translate ) {
    
    }
    // setTransformMode(CageEdit);
  }
}

void LayerItem::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
  std::cout << "LayerItem::mousePressEvent(): Processing..." << std::endl;
  {
    if ( event->button() != Qt::LeftButton ) {
     QGraphicsPixmapItem::mousePressEvent(event);
     return;
    }
    if ( m_trafomode == TransformMode::Translate || m_trafomode == TransformMode::Rotate || TransformMode::Scale ) {
      m_startPos = pos();
      m_startTransform = transform();
      if ( event->modifiers() & Qt::AltModifier )
        m_mode = TransformMode::Rotate;
      else if ( event->modifiers() & Qt::ControlModifier )
        m_mode = TransformMode::Scale;
      else
        m_mode = TransformMode::Translate;
      QGraphicsPixmapItem::mousePressEvent(event);
    } else {
      m_mode = TransformMode::None;
    }
  }
}
void LayerItem::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
  // std::cout << "LayerItem::mouseMoveEvent(): mode=" << m_mode << std::endl;
  {
    if ( m_trafomode == TransformMode::Translate ) {
      QGraphicsPixmapItem::mouseMoveEvent(event);
      return;
    }
    if ( m_mode == TransformMode::Rotate || m_mode == TransformMode::Scale ) {
        QPointF delta = event->scenePos() - event->buttonDownScenePos(Qt::LeftButton);
        QPointF c = boundingRect().center();
        QTransform t = m_startTransform;
        t.translate(c.x(), c.y());
        if ( m_mode == TransformMode::Rotate ) {
            t.rotate(delta.x());
        } else {
            qreal s = qMax(0.1, 1.0 + delta.y() * 0.005);
            t.scale(s, s);
        }
        t.translate(-c.x(), -c.y());
        setTransform(t);
        event->accept();
        return;
    }
  }
}

void LayerItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
  std::cout << "LayerItem::mouseReleaseEvent(): index=" << m_index << ", name=" << name().toStdString() << std::endl;
  {
    if ( !m_undoStack ) {
        QGraphicsPixmapItem::mouseReleaseEvent(event);
        return;
    }
    if ( m_mode == TransformMode::Translate ) {
        if ( pos() != m_startPos ) {
            m_undoStack->push(
                new MoveLayerCommand(this, m_startPos, pos(), m_index)
            );
        }
    } else if ( m_mode == TransformMode::Rotate || m_mode == TransformMode::Scale ) {
        if ( transform() != m_startTransform ) {
            QString name = m_mode == TransformMode::Rotate ? "Rotate Layer" : "Scale Layer";
            name += QString(" %1").arg(m_index);
            m_undoStack->push(
                new TransformLayerCommand(this, m_startPos, pos(), m_startTransform, transform(), name)
            );
        }
    }
    m_mode = None;
    QGraphicsPixmapItem::mouseReleaseEvent(event);
  }
}

// ------------------------ Handles ------------------------
void LayerItem::updateHandles() 
{
 std::cout << "LayerItem::updateHandles(): Processing..." << std::endl;
    qDeleteAll(m_handles);
    m_handles.clear();
    
    if ( !isSelected() )
        return;
    QRectF r = boundingRect();

    auto addScale = [&](QPointF p) {
        auto* h = new TransformHandleItem(this, TransformHandleItem::Scale);
        h->setParentItem(this);
        h->setPos(p);
        m_handles << h;
    };

    addScale(r.topLeft());
    addScale(r.topRight());
    addScale(r.bottomLeft());
    addScale(r.bottomRight());

    auto* rot = new TransformHandleItem(this, TransformHandleItem::Rotate);
    rot->setParentItem(this);
    rot->setPos(QPointF(r.center().x(), r.top() - 30));
    m_handles << rot;
}