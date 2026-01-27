#include "MirrorLayerCommand.h"

#include <QPainter>
#include <QtMath>

// -------------- history related methods  -------------- 
QJsonObject MirrorLayerCommand::toJson() const
{
   QJsonObject obj = AbstractCommand::toJson();
   obj["layerId"] = m_layerId;
   obj["mirrorPlane"] = m_mirrorPlane;
   obj["type"] = type();
   return obj;
}

MirrorLayerCommand* MirrorLayerCommand::fromJson( const QJsonObject& obj, const QList<LayerItem*>& layers )
{
    const int layerId = obj["layerId"].toInt(-1);
    LayerItem* layer = nullptr;
    for ( LayerItem* l : layers ) {
        if ( l->id() == layerId ) {
            layer = l;
            break;
        }
    }
    if ( !layer ) {
      qWarning() << "MirrorLayerCommand::fromJson(): Layer " << layerId << " not found.";
      return nullptr;
    }
    return new MirrorLayerCommand(
        layer,
        obj["layerId"].toInt(),
        obj["mirrorPlane"].toInt()
    );
}

