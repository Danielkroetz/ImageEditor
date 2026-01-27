#include "ImageView.h"
#include "../layer/LayerItem.h"
#include "../layer/MaskLayerItem.h"
#include "../undo/PaintCommand.h"
#include "../undo/CageWarpCommand.h"
#include "../undo/MaskStrokeCommand.h"
#include "../undo/PaintStrokeCommand.h"
#include "../undo/InvertLayerCommand.h"
#include "../undo/LassoCutCommand.h"
#include "../util/QImageUtils.h"
#include "../util/MaskUtils.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QWheelEvent>

#include <iostream>

/* ============================================================
 * Helper
 * ============================================================ */
static qreal pointToSegmentDist(const QPointF& p,
                                const QPointF& a,
                                const QPointF& b)
{
    const QPointF ab = b - a;
    const qreal t = std::clamp(
        QPointF::dotProduct(p - a, ab) / QPointF::dotProduct(ab, ab),
        0.0, 1.0
    );
    const QPointF proj = a + t * ab;
    return QLineF(p, proj).length();
}

static qreal distanceToPolygon(const QPointF& p, const QPolygonF& poly)
{
    qreal minDist = std::numeric_limits<qreal>::max();
    for (int i = 0; i < poly.size(); ++i) {
        QPointF a = poly[i];
        QPointF b = poly[(i + 1) % poly.size()];
        minDist = std::min(minDist, pointToSegmentDist(p, a, b));
    }
    return minDist;
}

/* ============================================================
 * Constructor
 * ============================================================ */
ImageView::ImageView( QWidget* parent ) : QGraphicsView(parent),
		m_parent(parent)
{   
    m_scene = new QGraphicsScene(this);
    // setup 
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    m_undoStack = new QUndoStack(this);
   /** 
    setDragMode(QGraphicsView::NoDrag);
    setMouseTracking(true);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
   */
}

// ------------------------ Self info -------------------------------------
void ImageView::printself() 
{
  std::cout << "ImageView::printself(): Info..." << std::endl;
}

// ------------------------ Update -------------------------------------
void ImageView::forcedUpdate()
{
  std::cout << "ImageView::forcedUpdate(): Processing..." << std::endl;
  {
    if ( m_selectedLayer != nullptr ) {
     m_selectedLayer->disableCage();
    } else {
     std::cout << " - no selected layer found" << std::endl;
    }
  }
}

// ------------------------ Mask layer -------------------------------------
void ImageView::createMaskLayer( const QSize& size )
{
   // std::cout << "ImageView::createMaskLayer(): size=" << size.width() << "x" << size.height() << std::endl;
   {
    // delete old mask
    if ( m_maskItem ) {
      QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "Please confirm operation", 
                     "A label mask already exists. Are you sure you want to create a new one? The old data will be completely deleted?",
                     QMessageBox::Yes | QMessageBox::No);
      if ( reply == QMessageBox::Yes ) {
        scene()->removeItem(m_maskItem);
        delete m_maskItem;
        m_maskItem = nullptr;
      } else {
       return;
      }
    }
    delete m_maskLayer;
    m_maskLayer = nullptr;
    // --- Mask data ---
    m_maskLayer = new MaskLayer(size);
    // --- Mask overlay ---
    m_maskItem = new MaskLayerItem(m_maskLayer);
    m_maskItem->setZValue(1000);
    m_maskItem->setOpacityFactor(0.4);
    // m_maskItem->setLabelColors(defaultMaskColors());
    scene()->addItem(m_maskItem);
   }
}

void ImageView::saveMaskImage( const QString& filename ) {
  if ( m_maskLayer != nullptr ) {
   QImage indexedImage = m_maskLayer->image().convertToFormat(QImage::Format_Indexed8);
   QList<QRgb> colorTable;
   colorTable.reserve(256);
   QVector<QColor> maskColors = defaultMaskColors();
   for ( const QColor &c : maskColors ) {
    colorTable.append(c.rgb());
   }
   for ( int i=maskColors.size() ; i < 256 ; ++i ) {
    colorTable.append(qRgb(i, 255 - i, 0));
   }
   indexedImage.setColorTable(colorTable);
   indexedImage.save(filename,"PNG");
  }
}

void ImageView::loadMaskImage( const QString& filename ) {
  QImage img(filename);
  if ( img.isNull() ) {
   qDebug() << "Fehler: Bild konnte nicht geladen werden!";
   return;
  }
  QSize size = baseLayer()->image().size();
  if ( img.size() != size ) {
   qDebug() << "Fehler: Size mismatch could not load file!";
   return;
  }
  if ( m_maskLayer != nullptr ) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Please confirm operation", 
                     "A label mask already exists. Are you sure you want to load a new one? The old data will be lost?",
                     QMessageBox::Yes | QMessageBox::No);
    if ( reply == QMessageBox::No ) {
      return;
    }
  } else {
   m_maskLayer = new MaskLayer(size);
   m_maskItem = new MaskLayerItem(m_maskLayer);
   m_maskItem->setZValue(1000);
   m_maskItem->setOpacityFactor(0.4);
   scene()->addItem(m_maskItem);
  }
  if ( img.format() != QImage::Format_Indexed8 ) {
   m_maskLayer->setImage(img.convertToFormat(QImage::Format_Grayscale8));
  } else {
   QImage grayImg(img.size(),QImage::Format_Grayscale8);
   for ( int y = 0; y < img.height(); ++y ) {
    const uchar* srcLine = img.scanLine(y);
    uchar* dstLine = grayImg.scanLine(y);
    memcpy(dstLine, srcLine, img.width());
   }
   m_maskLayer->setImage(grayImg);
  }
  viewport()->update();
}

void ImageView::setMaskTool( MaskTool t ) { 
 m_maskTool = t == m_maskTool ? MaskTool::None : t;
 if ( m_maskTool != MaskTool::None && m_maskLayer == nullptr ) {
    createMaskLayer(baseLayer()->image().size());
 }
}

void ImageView::setMaskCutTool( MaskCutTool t ) { 
 m_maskCutTool = t == m_maskCutTool ? MaskCutTool::Ignore : t;
}

// ------------------------ Layer tools -------------------------------------
void ImageView::centerOnLayer( Layer* layer ) {
 if ( !layer || !layer->m_item ) return;
 centerOn(layer->m_item); // QGraphicsView::centerOn
}

LayerItem* ImageView::currentLayer() const {
  auto items = m_scene->items(Qt::DescendingOrder);
  for ( QGraphicsItem* item : items ) {
    if ( auto* layer = dynamic_cast<LayerItem*>(item) )
      return layer;
  }
  return nullptr;
}

// ------------------------ Colortable tools -------------------------------------
void ImageView::setColorTable( const QVector<QRgb> &lut )
{
  std::cout << "ImageView::setColorTable(): Processing..." << std::endl;
  LayerItem* layer = currentLayer();
  if ( !layer ) return;
  m_undoStack->push(new InvertLayerCommand(layer,lut));
}

void ImageView::enablePipette( bool enabled ) {
    m_pipette = enabled;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
}

// ------------------------ Mouse tools -------------------------------------
void ImageView::mousePressEvent( QMouseEvent* event )
{
  // std::cout << "ImageView::mousePressEvent(): m_lassoEnabled=" << m_lassoEnabled << std::endl;
  
    if ( !scene() )
        return;     
        
    /* for debugging
     std::cout << "ImageView::mousePressEvent(): Scanning " << m_scene->items().size() << " items..." << std::endl;
     for ( auto* item : m_scene->items() ) {
       auto* layer = dynamic_cast<LayerItem*>(item);
       if ( layer ) {
        std::cout << "ImageView::mousePressEvent(): Found LayerItem " << layer->name().toStdString() << " ..." << std::endl;
       }
     }
     */
    // end

    if ( event->button() != Qt::LeftButton && event->button() != Qt::RightButton ) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    QPointF scenePos = mapToScene(event->pos());
    m_cursorPos = scenePos;

    // --- Shift-Pan: Bild verschieben ---
    if ( event->modifiers() & Qt::ShiftModifier ) {
        m_panning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return; // kein weiteres Event behandeln
    }
    
    // --- Check whether a layer has been clicked
    LayerItem* clickedItem = nullptr;
    auto itemsUnderCursor = m_scene->items(scenePos);
    // std::cout << " checking for layer click at position (" << scenePos.x() << ":" << scenePos.y() << ")..." << std::endl;
    for ( auto* item : itemsUnderCursor ) {
        auto* layer = dynamic_cast<LayerItem*>(item);
        if ( layer ) {
            // std::cout << "  found layer " << layer->name().toStdString() << std::endl;
            clickedItem = layer;
            break;
        }
    }
    if ( clickedItem ) {
        // Alle anderen Layer deselektieren
        for ( auto* item : m_scene->items() ) {
            auto* layer = dynamic_cast<LayerItem*>(item);
            if ( layer && layer != clickedItem )
                layer->setSelected(false);
        }
        clickedItem->setSelected(true); // roter Rahmen
        if ( clickedItem->isCageWarp() && clickedItem->cageMesh().isActive() ) {
         std::cout << " hello cage warp " << std::endl;
         m_activeLayer = clickedItem;
         m_selectedLayer = clickedItem;
         m_cageBefore = m_activeLayer->cageMesh().points();
        }
    }

    // --- Pipette ---
    if ( m_pipette ) {
        auto itemsUnderCursor = scene()->items(scenePos);
        for ( auto* item : itemsUnderCursor ) {
            auto* layer = dynamic_cast<LayerItem*>(item);
            if ( !layer ) continue;
            QPoint localPos = layer->mapFromScene(scenePos).toPoint();
            if ( !layer->image().rect().contains(localPos) ) continue;
            QColor color = layer->image().pixelColor(localPos);
            setBrushColor(color);
            emit pickedColorChanged(color);
            return;
        }
    }

    // --- Painting ---
    if ( m_paintToolEnabled ) {
        auto itemsUnderCursor = scene()->items(scenePos);
        for ( auto* item : itemsUnderCursor ) {
            auto* layer = dynamic_cast<LayerItem*>(item);
            if ( !layer ) continue;
            QPoint localPos = layer->mapFromScene(scenePos).toPoint();
            if ( !layer->image().rect().contains(localPos) ) continue;
            m_painting = true;
            m_paintLayer = layer;
            m_currentStroke.clear();
            m_currentStroke << localPos;
            //ALT: m_undoStack->push(new PaintStrokeCommand(layer, localPos, m_brushColor, m_brushRadius, m_brushHardness));
            layer->updateOriginalImage();
            layer->paintStrokeSegment(localPos,localPos,m_brushColor,m_brushRadius,m_brushHardness);
            viewport()->update();
            break;
        }
    }
    
    // --- Mask Painting ---
    if ( m_maskTool != MaskTool::None && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) ) {
        // NEW: m_maskStrokeActive = true;
        // NEW: m_currentMaskStroke.clear();
        m_maskPainting = true;
        m_maskStrokePoints.clear();
        m_maskStrokePoints << mapToScene(event->pos()).toPoint();
        return;
    }

    // --- Lasso starten ---
    if ( m_lassoEnabled ) {
        m_lassoPolygon.clear();
        m_lassoPolygon << scenePos.toPoint();
        if ( !m_lassoPreview  )
            m_lassoPreview = scene()->addPolygon(QPolygonF(m_lassoPolygon));
        else
            m_lassoPreview->setPolygon(QPolygonF(m_lassoPolygon));
        QRectF br = QPolygonF(m_lassoPolygon).boundingRect();
        if ( !m_lassoBoundingBox ) {
            QPen pen(Qt::green);
            pen.setWidth(0); // kosmetisch, 1px auf Bildschirm
            pen.setStyle(Qt::SolidLine);
            m_lassoBoundingBox = m_scene->addRect(br, pen);
            m_lassoBoundingBox->setZValue(1000);
        } else {
            m_lassoBoundingBox->setRect(br);
        }
        return; // fertig, kein weiteres Event
    }
    
    // --- Polygon starten ---
    if ( m_polygonEnabled ) {
    }

    QGraphicsView::mousePressEvent(event);
  
}

void ImageView::mouseMoveEvent( QMouseEvent* event )
{
    // std::cout << "ImageView::mouseMoveEvent(): m_painting=" << m_maskPainting << ", m_paintToolEnabled " << m_paintToolEnabled << std::endl;
    if ( !scene() )
        return;
        
    QPointF scenePos = mapToScene(event->pos());
    m_cursorPos = scenePos;

    emit cursorPositionChanged(int(scenePos.x()), int(scenePos.y()));
    emit scaleChanged(transform().m11());

    // --- Cursor-Farbe unter Maus ---
    bool colorFound = false;
    auto itemsUnderCursor = scene()->items(scenePos);
    for ( auto* item : itemsUnderCursor ) {
        auto* layer = dynamic_cast<LayerItem*>(item);
        if ( !layer ) continue;
        QPoint layerPos = layer->mapFromScene(scenePos).toPoint();
        if ( layer->image().rect().contains(layerPos) ) {
            QColor c = layer->image().pixelColor(layerPos);
            emit cursorColorChanged(c);
            colorFound = true;
            break;
        }
    }
    if ( !colorFound )
        emit cursorColorChanged(Qt::transparent);

    // --- Shift-Pan: nur wenn Shift gedrückt & linke Maustaste ---
    // std::cout << "shift-pan: processing: shift=" << (event->modifiers() & Qt::ShiftModifier) << "..." << std::endl;
    if ( (event->modifiers() & Qt::ShiftModifier) && (event->buttons() & Qt::LeftButton) ) {
        QPoint delta = event->pos() - m_lastMousePos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_lastMousePos = event->pos();
        return; // ❗ Qt bekommt KEIN Event
    }

    // --- Painting ---
    if ( m_painting && m_paintToolEnabled ) {
        auto itemsUnderCursor = m_scene->items(scenePos);
        for ( auto* item : itemsUnderCursor ) {
          auto* layer = dynamic_cast<LayerItem*>(item);
          if ( !layer || !layer->isVisible() ) continue;
          QPoint localPos = layer->mapFromScene(scenePos).toPoint();
          if ( !layer->image().rect().contains(localPos) ) continue;
          // PaintCommand jetzt auf Layer
          if ( m_currentStroke.isEmpty() || m_currentStroke.last() != localPos ) {
            m_currentStroke << localPos;
            layer->paintStrokeSegment(m_currentStroke[m_currentStroke.size()-2],localPos,m_brushColor,m_brushRadius,m_brushHardness);
            viewport()->update();
          }
          //ALT: m_undoStack->push(new PaintStrokeCommand(layer, localPos, m_brushColor, m_brushRadius, m_brushHardness));
          //ALT: viewport()->update();
          break; // nur oberstes Layer malen
        }
        return;
    }
    
    // --- Mask Painting ---
    if ( m_maskPainting && (event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton) ) {
     if ( m_maskLayer ) { // NEW: && m_maskStrokeActive
        // m_maskStrokePoints << mapToScene(event->pos()).toPoint();
        // m_maskItem->update();
        bool isRightButton = event->buttons() & Qt::RightButton ? true : false;
        int x = int(scenePos.x());
        int y = int(scenePos.y());
        if( !(x < 0 || y < 0 || x >= m_maskLayer->width() || y >= m_maskLayer->height()) ) {
         int r = m_maskBrushRadius;
         uchar newValue = isRightButton ? (m_maskTool == MaskTool::MaskErase ? m_currentMaskLabel : 0) : (m_maskTool == MaskTool::MaskErase ? 0 : m_currentMaskLabel);
         for( int dy = -r; dy <= r; ++dy ) {
          for( int dx = -r; dx <= r; ++dx ) {
            int px = x+dx;
            int py = y+dy;
            if( px<0 || py<0 || px>=m_maskLayer->width() || py>=m_maskLayer->height() )
                continue;
            if( dx*dx + dy*dy > r*r ) continue; // Kreis
            // NEW: uchar oldValue = m_maskLayer->pixel(x,y);
            // NEW: if ( oldValue == newValue ) continue;
            // NEW: m_currentMaskStroke.push_back({x,y,oldValue,newValue});
            m_maskLayer->setPixel(px, py, newValue);
          }
         }
         m_maskItem->maskUpdated();
         return;
        }
     }
    }

    // --- Lasso-Zeichnung ---
    if ( m_lassoEnabled && (event->buttons() & Qt::LeftButton) ) {
        m_lassoPolygon << scenePos.toPoint();
        if ( !m_lassoPreview )
            m_lassoPreview = scene()->addPolygon(QPolygonF(m_lassoPolygon));
        else
            m_lassoPreview->setPolygon(QPolygonF(m_lassoPolygon));
        QRectF br = QPolygonF(m_lassoPolygon).boundingRect();
        if ( !m_lassoBoundingBox ) {
            QPen pen(Qt::green);
            pen.setWidth(0); // kosmetisch
            pen.setStyle(Qt::SolidLine);
            m_lassoBoundingBox = m_scene->addRect(br, pen);
            m_lassoBoundingBox->setZValue(1000);
        } else {
            m_lassoBoundingBox->setRect(br);
        }
        return;
    }

    // --- Freie Auswahl (Path) ---
    if ( m_selecting ) {
        m_selectionPath.lineTo(scenePos);
        viewport()->update();
        return;
    }
    
    // Letzte Mausposition merken
    m_lastMousePos = event->pos();
    viewport()->update();
    
    // >>>
    QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mouseReleaseEvent( QMouseEvent* event )
{
  std::cout << "ImageView::mouseReleaseEvent(): Processing..." << std::endl;
  {
    if ( !scene() )
        return;
    // --- Cage warp beenden ---
    if ( event->button() == Qt::LeftButton && m_selectedCageLayer ) {
        QVector<QPointF> cageAfter = m_selectedCageLayer->cageMesh().points();
        std::cout << "ImageView::mouseReleaseEvent(): layer=" << m_selectedCageLayer->name().toStdString() << ": cageAfter=" << cageAfter.size() << ", cageBefore=" << m_cageBefore.size() << std::endl;
        // Prüfen, ob Cage verändert wurde
        if ( cageAfter != m_cageBefore ) {
         std::cout << " cage has been modified " << std::endl;
         if ( m_cageWarpCommand == nullptr ) {
          std::cout << "Creating new layer undo/redo instance..." << std::endl;
          int rows = m_selectedCageLayer->cageMesh().rows();
          int columns = m_selectedCageLayer->cageMesh().cols();
          m_cageWarpCommand = new CageWarpCommand(m_selectedLayer, m_cageBefore, cageAfter, m_selectedLayer->boundingRect(), rows, columns);
          m_undoStack->push(m_cageWarpCommand);
         } else {
          m_cageWarpCommand->pushNewStep();
         } 
         m_selectedCageLayer->applyTriangleWarp();
        }
        m_activeLayer = nullptr;
        m_cageBefore.clear();
    }
    // --- Lasso beenden ---
    if ( m_lassoEnabled && event->button() == Qt::LeftButton ) {
        if ( m_lassoPolygon.size() > 2 ) {
            createLassoLayer(); // erzeugt neues Layer aus Polygon
        }
        // Polygon entfernen
        if ( m_lassoPreview ) {
            scene()->removeItem(m_lassoPreview);
            delete m_lassoPreview;
            m_lassoPreview = nullptr;
        }
        // Bounding Box entfernen
        if ( m_lassoBoundingBox ) {
            scene()->removeItem(m_lassoBoundingBox);
            delete m_lassoBoundingBox;
            m_lassoBoundingBox = nullptr;
        }
        m_lassoPolygon.clear();
        return; // fertig, kein weiteres Event
    }
    // --- Painting beenden ---
    if ( m_painting && event->button() == Qt::LeftButton ) {
       std::cout << "stop painting of " <<  m_currentStroke.size() << " points..." << std::endl;
        if ( m_currentStroke.size() > 1 ) {
            m_undoStack->push(new PaintStrokeCommand(m_paintLayer,m_currentStroke,m_brushColor,
                                    m_brushRadius,m_brushHardness));
        }
        m_paintLayer = nullptr;
        m_currentStroke.clear();
        m_painting = false;
    }
    // --- Misc ---
    if ( event->button() == Qt::LeftButton ) {
        m_painting = false;
        // Shift-Pan beenden
        if ( m_panning ) {
            m_panning = false;
            setCursor(Qt::ArrowCursor);
        }
        // Freie Auswahl beenden
        if ( m_selecting ) {
            m_selecting = false;
            m_selectionPath.closeSubpath();
            viewport()->update();
        }
    }
    // --- Mask Painting beenden ---
    if ( m_maskPainting && (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) ) {
        m_maskStrokeActive = false;
        m_maskPainting = false;
        
        // NEW:
        if ( !m_currentMaskStroke.isEmpty() ) {
          m_undoStack->push(
            new MaskPaintCommand(
                m_maskLayer,
                std::move(m_currentMaskStroke)
            )
          );
        }
       /* **** crash ****
        quint8 label = m_maskEraser ? 0 : m_currentMaskLabel;
        m_undoStack->push(
            new MaskStrokeCommand(
                m_maskLayer,
                m_maskStrokePoints,
                label,
                m_brushRadius
            )
        );
        */
        
        return;
    } 
    QGraphicsView::mouseReleaseEvent(event);
  }
}

void ImageView::wheelEvent( QWheelEvent* event )
{
    if ( !scene() || scene()->items().isEmpty() ) {
      event->ignore();
      return;
    }
    constexpr qreal zoomFactor = 1.15;
    qreal factor = (event->angleDelta().y() > 0) ? zoomFactor : 1.0 / zoomFactor;
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    scale(factor, factor);
    event->accept();
}

// --------------------------------- drawing ---------------------------------
void ImageView::drawForeground( QPainter* painter, const QRectF& )
{

    if ( !scene() )
        return;
        
    painter->save();

    // ----------------------------
    // LASSO
    // ----------------------------
    if ( m_selecting ) {
        QPen pen(Qt::white, 0, Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(m_selectionPath);
    }

    // ----------------------------
    // CROSSHAIR
    // ----------------------------
    if ( m_crosshairVisible ) {
        QPen pen(Qt::red, 0);   // 0 = kosmetisch
        painter->setPen(pen);
        QRectF r = scene()->sceneRect();
        painter->drawLine(QPointF(m_cursorPos.x(), r.top()),
                          QPointF(m_cursorPos.x(), r.bottom()));
        painter->drawLine(QPointF(r.left(), m_cursorPos.y()),
                          QPointF(r.right(), m_cursorPos.y()));
    }

    // ----------------------------
    // BRUSH-VORSCHAU
    // ----------------------------
    if ( m_showBrushPreview && m_paintToolEnabled ) {
        QPen outerPen(Qt::green, 0);
        outerPen.setCosmetic(true);
        painter->setPen(outerPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(m_cursorPos, m_brushRadius, m_brushRadius);
        // Hardness-Ring
        if ( m_brushHardness > 0.0 ) {
            qreal innerRadius = m_brushRadius * m_brushHardness;
            QPen innerPen(Qt::green, 0, Qt::DashLine);
            innerPen.setCosmetic(true);
            painter->setPen(innerPen);
            painter->drawEllipse(m_cursorPos, innerRadius, innerRadius);
        }
    }
    
    painter->restore();
    
}

// ---------------------------- Selectior -----------------------------
void ImageView::clearSelection()
{
    m_selectionPath = QPainterPath();
    viewport()->update();
}

void ImageView::cutSelection()
{
    if ( m_selectionPath.isEmpty() )
        return;

    auto* layer = dynamic_cast<LayerItem*>(m_scene->focusItem());
    if ( !layer ) return;

    QImage& srcImage = layer->image();
    if ( srcImage.isNull() ) return;

    QRectF sceneRect = m_selectionPath.boundingRect();
    QRect imageRect = layer->mapFromScene(sceneRect).boundingRect().toRect()
                          .intersected(srcImage.rect());
    if ( imageRect.isEmpty() ) return;

    QImage cutImage(imageRect.size(), QImage::Format_ARGB32_Premultiplied);
    cutImage.fill(Qt::transparent);

    {
        QPainter p(&cutImage);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath localPath = layer->mapFromScene(m_selectionPath);
        p.translate(-imageRect.topLeft());
        p.setClipPath(localPath);
        p.drawImage(0, 0, srcImage,
                    imageRect.x(), imageRect.y(),
                    imageRect.width(), imageRect.height());
    }

    {
        QPainter p(&srcImage);
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        QPainterPath localPath = layer->mapFromScene(m_selectionPath);
        p.drawPath(localPath);
    }

    layer->updatePixmap();

    auto* newLayer = new LayerItem(QPixmap::fromImage(cutImage));
    newLayer->setPos(layer->mapToScene(imageRect.topLeft()));
    newLayer->setFlags(QGraphicsItem::ItemIsMovable |
                       QGraphicsItem::ItemIsSelectable |
                       QGraphicsItem::ItemIsFocusable);

    m_scene->addItem(newLayer);
    newLayer->setSelected(true);
    newLayer->setFocus();

    clearSelection();
}

// ---------------------------- Layer methods -----------------------------
void ImageView::setLayerTransformMode( LayerItem::TransformMode mode )
{
   // std::cout << "ImageView::setLayerTransformMode(): mode=" << mode << std::endl;
   {
	for ( auto* item : m_scene->items() ) {
        auto* layer = dynamic_cast<LayerItem*>(item);
        if ( layer && layer->isSelected() ) {
          layer->setTransformMode(mode);
        }
    }
   }
}

void ImageView::setNumberOfCageControlPoints( int nControlPoints ) 
{
  // std::cout << "ImageView::setNumberOfCageControlPoints(): nControlPoints=" << nControlPoints << std::endl;
  {
    for ( auto* item : m_scene->items(Qt::DescendingOrder) ) {
      auto* layer = dynamic_cast<LayerItem*>(item);
      if ( layer && layer->getType() != LayerItem::MainImage ) {
        if ( layer->hasActiveCage() ) {
         layer->setNumberOfActiveCagePoints(nControlPoints);
         return;
        }
      }
    }
  }
}

void ImageView::setCageWarpRelaxationSteps( int nRelaxationSteps )
{
  // std::cout << "ImageView::setCageWarpRelaxationSteps(): nRelaxationSteps=" << nRelaxationSteps << std::endl;
  {
    for ( auto* item : m_scene->items(Qt::DescendingOrder) ) {
      auto* layer = dynamic_cast<LayerItem*>(item);
      if ( layer && layer->getType() != LayerItem::MainImage ) {
        if ( layer->hasActiveCage() ) {
         layer->setCageWarpRelaxationSteps(nRelaxationSteps);
         return;
        }
      }
    }
  }
}

LayerItem* ImageView::baseLayer()
{
    for ( auto* item : m_scene->items(Qt::DescendingOrder) ) {
      auto* layer = dynamic_cast<LayerItem*>(item);
      if ( layer && layer->getType() == LayerItem::MainImage ) {
        return layer;
      }  
    }
    return nullptr;
}

void ImageView::clearLayers()
{
}

void ImageView::createLassoLayer()
{
    std::cout << "ImageView::createLassoLayer(): polygon_size=" << m_lassoPolygon.size() << std::endl;
    // --- get main image layer ---
    LayerItem* base = baseLayer();
    if ( !base || m_lassoPolygon.size() < 3 )
        return;
    QImage& src = base->image();
    // --- prepare ---
    QPolygonF polyF = QPolygonF(m_lassoPolygon.begin(), m_lassoPolygon.end());
    QRectF boundsF = polyF.boundingRect();
    QRect bounds = boundsF.toAlignedRect(); // ganzzahlig
    QImage backup = src.copy(bounds);
    // --- create mask ---
    QImage mask(bounds.size(), QImage::Format_Alpha8);
    mask.fill(0);
    QPainter pm(&mask);
    pm.setRenderHint(QPainter::Antialiasing);
    pm.setBrush(Qt::white);
    pm.setPen(Qt::NoPen);
    // Polygon relativ zur Bounding-Box verschieben
    QPolygonF relativePoly = polyF;
    for ( int i=0; i<relativePoly.size(); ++i )
      relativePoly[i] -= bounds.topLeft();
    pm.drawPolygon(relativePoly);
    pm.end();
    // --- lasso fear ---
    if ( m_lassoFeatherRadius > 0 ) {
      mask = QImageUtils::blurAlphaMask(mask,m_lassoFeatherRadius);
    }
    // --- cut layer ---
    QImage cut(bounds.size(), QImage::Format_ARGB32_Premultiplied);
    cut.fill(Qt::transparent);
    if ( m_maskCutTool == MaskCutTool::Mask && m_maskLayer != nullptr ) {
     for ( int y=0; y<bounds.height(); ++y ) {
      uchar* m = mask.scanLine(y);
      unsigned int ypos = bounds.top() + y;
      for ( int x=0; x<bounds.width(); ++x ) {
       unsigned int xpos = bounds.left() + x;
       QPoint imgPos(xpos, ypos);
       QColor c = src.pixelColor(imgPos);
       if ( c != m_backgroundColor && m[x] > 0 && m_maskLayer->pixel(xpos,ypos) == 0 ) {
         c.setAlpha(m[x]); 
         cut.setPixelColor(x,y,c);
         if ( m[x] == 255 )
            src.setPixelColor(imgPos, m_backgroundColor);
       }
      }
     }
    } else if ( m_maskCutTool == MaskCutTool::OnlyMask && m_maskLayer != nullptr ) {
     for ( int y=0; y<bounds.height(); ++y ) {
      uchar* m = mask.scanLine(y);
      unsigned int ypos = bounds.top() + y;
      for ( int x=0; x<bounds.width(); ++x ) {
       unsigned int xpos = bounds.left() + x;
       QPoint imgPos(xpos, ypos);
       QColor c = src.pixelColor(imgPos);
       if ( c != m_backgroundColor && m[x] > 0 && m_maskLayer->pixel(xpos,ypos) != 0 ) {
         c.setAlpha(m[x]); 
         cut.setPixelColor(x,y,c);
         if ( m[x] == 255 )
            src.setPixelColor(imgPos, m_backgroundColor);
       }
      }
     }
    } else {
     for ( int y=0; y<bounds.height(); ++y ) {
      uchar* m = mask.scanLine(y);
      unsigned int ypos = bounds.top() + y;
      for ( int x=0; x<bounds.width(); ++x ) {
        QPoint imgPos(bounds.left() + x, ypos);
        QColor c = src.pixelColor(imgPos);
        if ( c != m_backgroundColor && m[x] > 0 ) {
         c.setAlpha(m[x]); 
         cut.setPixelColor(x,y,c);
         //if ( m[x] > 0 )
           //  src.setPixelColor(imgPos, m_backgroundColor);
        }
      }
     }
    }
    // --- Neues LayerItem ---
    int nidx = m_layers.size()+1;
    Layer* layer = new Layer(nidx,cut);
    layer->m_name = QString("Lasso Layer %1").arg(nidx);
    LayerItem* newLayer = new LayerItem(cut); // QPixmap::fromImage(cut));
    newLayer->setParent(m_parent);
    newLayer->setIndex(nidx);
    newLayer->setLayer(layer);
    newLayer->setUndoStack(m_undoStack);
    LassoCutCommand* cmd = new LassoCutCommand(base, newLayer, bounds, cut, nidx); // hier schon ausgeschnitten
    m_undoStack->push(cmd);
    newLayer->setPos(base->mapToScene(bounds.topLeft()));
    newLayer->setZValue(base->zValue()+1);
    newLayer->setSelected(true);
    // newLayer->setShowGreenBorder(true); // falls vorhanden
    m_scene->addItem(newLayer);
    base->updatePixmap();
    layer->m_item = newLayer;
    m_layers.push_back(layer);
    // --- send signal to mainWindow ---
    emit lassoLayerAdded();
    
    
/*
    if ( m_lassoPolygon.size() < 3 ) return; // zu klein

    // --- 1. Polygon & BoundingRect ---
    QPolygon lassoPoly(m_lassoPolygon);
    QRectF bounds = lassoPoly.boundingRect();

    // --- 2. Polygon & BoundingRect ---
    LayerItem* mainLayer = nullptr;
    for ( auto* item : m_scene->items(Qt::DescendingOrder) ) {
      auto* layer = dynamic_cast<LayerItem*>(item);
      if ( layer && layer->getType() == LayerItem::MainImage ) {
        mainLayer = layer;
        break;
      }  
    }
    if ( !mainLayer ) return;
    QImage sourceImage = mainLayer->pixmap().toImage(); // Hauptbild
    
    // --- 3. New pixmap for Lasso-Layer ---
    QImage newImage(bounds.size().toSize(), QImage::Format_ARGB32);
    newImage.fill(Qt::transparent);
    QPainter p(&newImage);
    QRegion clipRegion(lassoPoly.translated(-bounds.topLeft().toPoint()));
    p.setClipRegion(clipRegion);
    p.drawImage(-bounds.topLeft(), sourceImage);
    p.end();
    QPixmap pixmap = QPixmap::fromImage(newImage);

    // --- 3. Create new LayerItem ---
    Layer* layer = new Layer;
    layer->name = "Lasso Layer";
    LayerItem* item = new LayerItem(pixmap); // LassoLayer
    item->setLayer(layer);
    item->setUndoStack(m_undoStack);
    item->setPos(bounds.topLeft()); // Position setzen
    m_scene->addItem(item);
    item->setSelected(true);
    layer->item = item;
    m_layers.push_back(layer);
    
    // UndoStack kann hier auch ergänzt werden (optional)
    emit layerAdded();
*/
    
/*
    if ( m_lassoPolygon.size() < 3 ) return;
    QRect bounding = QPolygonF(m_lassoPolygon).boundingRect().toAlignedRect();
    QImage newLayer(bounding.size(), QImage::Format_ARGB32_Premultiplied);
    newLayer.fill(Qt::transparent);
    for (int y = bounding.top(); y < bounding.bottom(); ++y) {
        for (int x = bounding.left(); x < bounding.right(); ++x) {
            QPoint pt(x,y);
            if (QPolygonF(m_lassoPolygon).containsPoint(pt, Qt::OddEvenFill)) {
                if (x>=0 && x<m_image.width() && y>=0 && y<m_image.height()) {
                    newLayer.setPixelColor(x-bounding.left(),
                                           y-bounding.top(),
                                           m_image.pixelColor(x,y));
                    m_image.setPixelColor(x,y,Qt::transparent);
                }
            }
        }
    }
    Layer* layer = new Layer;
    layer->name = "Lasso Layer";
    layer->image = newLayer;
    LayerItem* item = new LayerItem(QPixmap::fromImage(newLayer));
    item->setLayer(layer);
    item->setFlags(QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemSendsGeometryChanges);
    item->setPos(bounding.topLeft());
    // item->setShowBoundingBox(true);
    m_scene->addItem(item);
    layer->item = item;
    m_layers.push_back(layer);
    viewport()->update();
    emit layerAdded();
*/
}