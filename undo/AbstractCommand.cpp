#include "AbstractCommand.h"

// Include of all relevant commands
#include "CageWarpCommand.h"
#include "MoveLayerCommand.h"
#include "PaintStrokeCommand.h"
#include "InvertLayerCommand.h"
#include "LassoCutCommand.h"
#include "TransformLayerCommand.h"
#include "EditablePolygonCommand.h"

// >>>
#include <QDebug>

AbstractCommand::AbstractCommand( QUndoCommand* parent )
    : QUndoCommand(parent)
{
}

/**
 * @brief Factory zur Rekonstruktion eines Commands aus JSON
 *
 * Wichtig:
 * - Reihenfolge im JSON = Ausführungsreihenfolge
 * - redo() wird beim push() automatisch ausgeführt
 */
AbstractCommand* AbstractCommand::fromJson( const QJsonObject& obj, ImageView* view )
{
    if ( !obj.contains("type") ) {
        qWarning() << "AbstractCommand::fromJson(): Missing type.";
        return nullptr;
    }
    const QString type = obj["type"].toString();
    // ---- Dispatch nach Command-Typ ----
    // ---- ------------------------- ----
    qWarning() << "AbstractCommand::fromJson(): Unprocessed command type: " << type;
    return nullptr;
}

AbstractCommand* AbstractCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers )
{
    if ( !obj.contains("type") ) {
      qWarning() << "AbstractCommand::fromJson(): Missing type.";
      return nullptr;
    }
    const QString type = obj["type"].toString();
    // ---- Dispatch nach Command-Typ ----
    if ( type == "MoveLayer" )
        return MoveLayerCommand::fromJson(obj,layers);
    if ( type == "Paint" )
        return nullptr;
    if ( type == "LassoCut" )
        return nullptr;
    if ( type == "PaintStroke" )
        return PaintStrokeCommand::fromJson(obj,layers);
    if ( type == "InvertLayer" )
        return InvertLayerCommand::fromJson(obj,layers);
    if ( type == "CageWarp" )
        return CageWarpCommand::fromJson(obj,layers);
    if ( type == "LassoCut" )
        return LassoCutCommand::fromJson(obj,layers);
    if ( type == "TransformLayer" )
    	return TransformLayerCommand::fromJson(obj,layers);
    if ( type == "EditablePolygonCommand" )
        return EditablePolygonCommand::fromJson(obj,layers);
    qWarning() << "AbstractCommand::fromJson(): Unprocessed command type: " << type;
    return nullptr;
}

/**
 */
LayerItem* AbstractCommand::getLayerItem( const QList<LayerItem*>& layers, int layerId )
{
    LayerItem* layer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == layerId ) {
            layer = l;
            break;
        }
    }
    return layer;
}
