#include "AbstractCommand.h"

class CreatePolygonMaskCommand : public AbstractCommand
{
public:
    CreatePolygonMaskCommand(QGraphicsScene* scene,
                             EditablePolygon* poly,
                             const QImage& baseImage)
        : AbstractCommand("Create Polygon Mask")
        , m_scene(scene), m_poly(poly), m_baseImage(baseImage)
    {
    }

    void redo() override
    {
        if (!m_item) {
            m_item = new PolygonMaskLayerItem(m_poly, m_baseImage);
        }
        m_scene->addItem(m_item);
    }

    void undo() override
    {
        if (m_item) {
            m_scene->removeItem(m_item);
        }
    }

private:
    QGraphicsScene* m_scene;
    EditablePolygon* m_poly;
    QImage m_baseImage;
    PolygonMaskLayerItem* m_item = nullptr;
};
