#pragma once

#include <QUndoCommand>
#include <QJsonObject>
#include <QString>

class LayerItem;
class ImageView;

/**
 * @brief Basisklasse für alle serialisierbaren Undo/Redo Commands
 *
 * - Erweitert QUndoCommand
 * - Erzwingt JSON-Serialisierung
 * - Ermöglicht Rekonstruktion nach Neustart
 */
class AbstractCommand : public QUndoCommand
{

public:

    explicit AbstractCommand( QUndoCommand* parent = nullptr );
    virtual ~AbstractCommand() override = default;

    // ---- Serialisierung ----
    virtual QString type() const = 0;
    virtual QJsonObject toJson() const {
        QJsonObject obj;
        obj["text"] = text();
        return obj;
    };

    // ---- Factory ----
    static AbstractCommand* fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers );
    static AbstractCommand* fromJson( const QJsonObject& obj, ImageView* view );
    
    // --- Static Helper ---
    static LayerItem* getLayerItem( const QList<LayerItem*>& layers, int layerId = 0 );
    
};
