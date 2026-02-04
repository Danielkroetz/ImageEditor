#pragma once

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QTransform>
#include <QImage>
#include <QJsonObject>
#include <QJsonArray>

class PerspectiveTransform : public QObject
{
    Q_OBJECT

public:

    enum Constraint {
        None            = 0,
        KeepInBounds    = 1 << 0,
        OrthogonalEdges = 1 << 1
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)
    
    void setConstraints( Constraints c ) { m_constraints = c; }
    Constraints constraints() const { return m_constraints; }
    
    void lockPoint( int idx, bool locked );
    bool isPointLocked( int idx ) const;
    void setTargetPoint( int idx, const QPointF& p );

    explicit PerspectiveTransform( QObject* parent = nullptr );

    // Source = ursprüngliche Ecken (BoundingBox)
    void setSourceQuad( const QVector<QPointF>& src );
    void setTargetQuad( const QVector<QPointF>& dst );

    const QVector<QPointF>& sourceQuad() const { return m_src; }
    const QVector<QPointF>& targetQuad() const { return m_dst; }

    bool isValid() const;

    QTransform transform() const;

    QImage apply( const QImage& srcImage ) const;

    // Serialization
    QJsonObject toJson() const;
    static PerspectiveTransform fromJson( const QJsonObject& );

signals:
    void changed();

private:

    void applyConstraints(int idx);

    QVector<QPointF> m_src;
    QVector<QPointF> m_dst;

    QSet<int> m_lockedPoints;
    Constraints m_constraints;
    
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PerspectiveTransform::Constraints)
