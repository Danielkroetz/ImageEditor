class PerspectiveHandleItem : public QGraphicsEllipseItem
{
public:
    PerspectiveHandleItem(int index,
                          PerspectiveTransform* transform);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;

private:
    int m_index;
    PerspectiveTransform* m_transform;
};
