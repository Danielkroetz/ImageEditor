#pragma once

#include "LayerItem.h"

class CageControlPointItem;

class CageLayerItem : public LayerItem
{

public:
    CageLayerItem( const QPixmap& pix, const QPolygonF& cagePolygon = QPolygonF() );

    void setCagePoint( int idx, const QPointF& pos );
    void beginCageEdit();
    void endCageEdit();
    
    bool isEditing() const { return m_editing; }
    
    void enableCage();
    void disableCage();
    bool cageEnabled() const;

protected:
    void paint( QPainter*, const QStyleOptionGraphicsItem*, QWidget* ) override;

private:
    QVector<QPointF> m_cage;
    QVector<CageControlPointItem*> m_points;
    QTransform m_startTransform;
    bool m_cageEnabled = false;
    bool m_editing = false;
    void createDefaultBoundingBox();
    
};
