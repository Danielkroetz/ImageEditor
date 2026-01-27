#pragma once

#include <QUndoCommand>
#include <QImage>
#include <QColor>
#include <QPoint>

class LayerItem; // Vorwärtsdeklaration

class PaintCommand : public QUndoCommand
{
public:
    // Konstruktor: malt auf das gegebene Layer an der Position pos
    PaintCommand(LayerItem* layer,
                 const QPoint& pos,
                 const QColor& color,
                 int radius,
                 qreal hardness = 1.0);

    void undo() override;
    void redo() override;

private:
    LayerItem* m_layer;   // Layer, auf dem gemalt wird
    QImage*    m_image;   // Pointer auf das Bild im Layer
    QPoint     m_pos;     // Position des Pinsels
    QColor     m_color;   // Pinsel-Farbe
    int        m_radius;  // Pinsel-Radius
    qreal      m_hardness;// Pinsel-Härte 0..1

    QImage     m_backup;  // Backup der bearbeiteten Region für Undo
};