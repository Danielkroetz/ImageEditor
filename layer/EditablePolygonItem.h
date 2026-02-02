#pragma once

#include <QGraphicsObject>
#include <QPolygonF>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

#include "EditablePolygon.h"
#include "LayerItem.h"

class EditablePolygonItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit EditablePolygonItem( EditablePolygon* poly, QGraphicsItem* parent = nullptr );

    // --- QGraphicsItem ---
    QRectF boundingRect() const override;
    void paint( QPainter* p,
               const QStyleOptionGraphicsItem* opt,
               QWidget* widget ) override;

    // --- Interaktion ---
    void pointMoved( int idx, const QPointF& scenePos );

signals:
    // → wird vom ImageView abgefangen und in UndoCommands umgesetzt
    void requestMovePoint( int idx, const QPointF& from, const QPointF& to );
    void requestInsertPoint( int idx, const QPointF& scenePos );
    void requestRemovePoint( int idx );

protected:
    // --- Maus-Events ---
    void mousePressEvent( QGraphicsSceneMouseEvent* e ) override;
    void mouseMoveEvent( QGraphicsSceneMouseEvent* e ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* e ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* e ) override;

private slots:
    void updateGeometry();
    void onVisibilityChanged();

private:
    // --- Hilfsfunktionen ---
    int hitTestPoint( const QPointF& scenePos ) const;
    int hitTestEdge( const QPointF& scenePos ) const;
    void rebuildHandles();

private:

    EditablePolygon* m_poly = nullptr;
    LayerItem* m_layer = nullptr;

    // Dragging-State
    int     m_activePoint = -1;
    QPointF m_dragStartPos;
    QPointF m_dragMousePressPos;

    // Darstellung
    QVector<QGraphicsEllipseItem*> m_handles;
    qreal   m_handleRadius = 4.0;

    // Styling
    QColor m_lineColor   = QColor(0, 255, 0);
    QColor m_fillColor   = QColor(0, 255, 0, 40);
    QColor m_handleColor = QColor(255, 0, 0);

    bool m_editable = true;
};
