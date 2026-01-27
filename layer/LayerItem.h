#pragma once

#include <QGraphicsPixmapItem>
#include <QUndoStack>
#include <QTransform>
#include <QJsonArray>
#include <QImage>
#include <QPen>

#include "CageMesh.h"

// ---
class Layer;
class CageOverlayItem;
class CageControlPointItem;
class TransformHandleItem;

// ---
class LayerItem : public QGraphicsPixmapItem
{

public:

    enum LayerType { MainImage, LassoLayer };
    enum TransformMode { None, CageEdit, CageWarp, Translate, Rotate, Scale, Flip, Flop, Perseptive };

    LayerItem( const QPixmap& pixmap, QGraphicsItem* parent = nullptr );
    LayerItem( const QImage& image, QGraphicsItem* parent = nullptr );
    
    void paintStrokeSegment( const QPoint& p0, const QPoint &p1, const QColor &color, int radius, float hardness );

    QImage& image( int id=0 );
    const QImage& originalImage();
    void updatePixmap();
    void setFileInfo( const QString& filePath );
    void setImage( const QImage &image );
    void setLayer( Layer *layer );
    QString filename() const { return m_filename; }
    QString checksum() const { return m_checksum; }
    QString name() const;
    
    void setMirror( int plane );
    void setImageTransform( const QTransform& transform, bool combine = false );
    // void setCagePoint( int idx, const QPointF& pos );
    void endCageEdit( int idx, const QPointF& pos );
    void setType( LayerType layerType );
    void updateCagePoint( TransformHandleItem*, const QPointF& localPos );
    void commitCageTransform( const QVector<QPointF> &cage );
    void beginCageEdit();
    // void enableCage();
    void disableCage();
    bool cageEnabled() const;
    
    int id() const { return m_index; }
    bool isEditing() const { return m_cageEditing; }
    bool isCageWarp() const { return m_trafomode == TransformMode::CageWarp ? true : false; }
    
    QUndoStack* undoStack() const { return m_undoStack; }
    void setParent( QWidget *parent ) { m_parent = parent; }
    void setUndoStack( QUndoStack* stack ) { m_undoStack = stack; }
    void setIndex( const int index ) { m_index = index; }
    void setName( const QString& name ) { m_name = name; }
    LayerType getType() const { return m_type; }
    bool hasActiveCage() const { return m_cageEnabled; }
    
    void setNumberOfActiveCagePoints( int nControlPoints );
    void setCageWarpRelaxationSteps( int nRelaxationsSteps );
    
    void setTransformMode( TransformMode mode );
    TransformMode transformMode() const { return m_mode; }
    
    const CageMesh& cageMesh() const { return m_mesh; }
    void setCagePoint( int idx, const QPointF& pos );
    void setCagePoints( const QVector<QPointF>& pts );
    QVector<QPointF> cagePoints() const;
    void applyTriangleWarp();
    void applyCageWarp();
    void enableCage( int, int );
    
    void updateHandles();
    void updateOriginalImage();
    void updateImageRegion( const QRect& rect );
    
    void printself();

protected:

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;
    
    void mousePressEvent( QGraphicsSceneMouseEvent* ) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* ) override;
    
    QImage m_image;
    QImage m_originalImage;
    
    QPointF m_startPos;
    
private:

    void init();
    
    int m_index = 0;
    int m_nCageWarpRelaxationsSteps = 0;
    
    TransformMode m_mode = None;
    TransformMode m_trafomode = LayerItem::Translate;;
    LayerType m_type = LayerItem::LassoLayer;
    
    QString m_name = "";
    QString m_filename = "";
    QString m_checksum = "";
    QVector<QGraphicsItem*> m_handles;
    QVector<QPointF> m_originalCage;
    QVector<QPointF> m_cage;
    // QVector<CageControlPointItem*> m_points;
    
    CageMesh m_mesh;
    CageOverlayItem* m_cageOverlay = nullptr;

	Layer* m_layer = nullptr;
	
	bool m_nogui = false; 
	bool m_lockToBoundingBox = true;
	bool m_showBoundingBox = true;
	bool m_dragging = false;
	bool m_cageEnabled = false;
	bool m_cageEditing = false;
	
	QPen m_lassoPen;
	QPen m_selectedPen;
	
	QPointF m_pressScenePos;
    QTransform m_startTransform;
	
	QWidget* m_parent = nullptr;
	QUndoStack* m_undoStack = nullptr;
	
};