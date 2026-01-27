#pragma once

#include <QVector>
#include <QPointF>
#include <QRectF>

struct CageSpring {
    int a;
    int b;
    qreal restLength;
};

/*
 * CageMesh
 * --------
 * Repräsentiert ein regelmäßiges 2D-Gitter über einem Layer.
 * - speichert Original- und aktuelle Punkte
 * - verhindert Punktüberlappungen (Spring-Constraint)
 * - Grundlage für Piecewise-Affine-Warp (Triangle Warp)
 */
 
class CageMesh
{
public:
    CageMesh();

    void create( const QRectF& bounds, int cols, int rows );
    void update( int cols, int rows );
    
    int cols() const { return m_cols; }
    int rows() const { return m_rows; }
    
    const QVector<QPointF>& points() const { return m_points; }
    const QVector<QPointF>& originalPoints() const { return m_originalPoints; }

    int pointCount() const { return m_points.size(); }
    bool isActive() const { return m_active; }
    void setActive( bool );

    
    QPointF originalPoint( int i ) const;
    QPointF point( int i ) const;

    void setPoint( int i, const QPointF& pos );
    void relax( int iterations = 5 );
    
    void setPoints( const QVector<QPointF>& pts );
    void enforceConstraints( int idx );
    QRectF boundingRect() const;

private:

    bool m_active = true;
    
    int m_cols = 0;
    int m_rows = 0;
    
    QVector<QPointF> m_points;
    QVector<QPointF> m_originalPoints;
    QVector<CageSpring> m_springs;
    
    qreal m_minSpacing = 1.0;

    void addSpring( int a, int b );
};
