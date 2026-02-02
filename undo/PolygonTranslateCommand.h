#pragma once

#include "AbstractCommand.h"
#include "../layer/EditablePolygon.h"

class PolygonTranslateCommand : public AbstractCommand
{

public:
    PolygonTranslateCommand( EditablePolygon* poly, const QPointF& start, const QPointF& end,
                            QUndoCommand* parent = nullptr );

    QString type() const override { return "TranslatePolygon"; }
    
    void undo() override;
    void redo() override;

    QJsonObject toJson() const override;
    static PolygonTranslateCommand* fromJson( const QJsonObject& obj, EditablePolygon* poly );

private:

    EditablePolygon* m_poly;
    
    bool m_redo = false;
    QPointF m_start;
    QPointF m_end;
    
};