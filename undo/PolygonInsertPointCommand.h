#pragma once

#include "AbstractCommand.h"
#include "EditablePolygon.h"

class PolygonInsertPointCommand : public AbstractCommand
{
public:
    PolygonInsertPointCommand(EditablePolygon* poly,
                              int idx,
                              const QPointF& p,
                              QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonInsertPointCommand* fromJson(const QJsonObject& obj,
                                               EditablePolygon* poly);

private:
    EditablePolygon* m_poly;
    int m_idx;
    QPointF m_point;
};
