PerspectiveHandleItem::PerspectiveHandleItem(
    int index,
    PerspectiveTransform* transform)
    : QGraphicsEllipseItem(-5, -5, 10, 10)
    , m_index(index)
    , m_transform(transform)
{
    setBrush(Qt::red);
    setFlag(ItemIsMovable);
    setFlag(ItemSendsScenePositionChanges);
}

void PerspectiveHandleItem::mouseMoveEvent( QGraphicsSceneMouseEvent* e )
{
    m_transform->setTargetPoint(m_index, e->scenePos());
    /*
    auto dst = m_transform->targetQuad();
    dst[m_index] = scenePos();
    m_transform->setTargetQuad(dst);
    QGraphicsEllipseItem::mouseMoveEvent(e);
    */
}
