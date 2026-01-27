cmake_minimum_required(VERSION 3.22)

project(ImageEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 finden
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Quellen
set(SOURCES
    main.cpp
    core/ImageLoader.cpp
    core/ImageProcessor.cpp
    gui/MainWindow.cpp
    gui/ImageView.cpp
    layer/LayerItem.cpp
    layer/CageLayerItem.cpp
    layer/CageMesh.cpp
    layer/CageControlPointItem.cpp
    layer/CageOverlayItem.cpp
    layer/EditablePolygon.cpp
    layer/EditablePolygonItem.cpp
    layer/TransformHandleItem.cpp
    layer/MaskLayer.cpp
    layer/MaskLayerItem.cpp
    undo/AbstractCommand.cpp
    undo/CageMoveCommand.cpp
    undo/CageWarpCommand.cpp
    undo/LassoCutCommand.cpp
    undo/PaintCommand.cpp
    undo/PaintStrokeCommand.cpp
    undo/InvertLayerCommand.cpp
    undo/MirrorLayerCommand.cpp
    undo/TransformLayerCommand.cpp
    undo/MoveLayerCommand.cpp
    undo/MaskStrokeCommand.cpp
    undo/MaskPaintCommand.cpp
)

set(HEADERS
    core/ImageLoader.h
    core/ImageProcessor.h
    gui/MainWindow.h
    gui/ImageView.h
    layer/LayerItem.h
    layer/CageLayerItem.h
    layer/CageMesh.h
    layer/CageControlPointItem.h
    layer/CageOverlayItem.h
    layer/LassoCutCommand.h
    layer/EditablePolygon.h
    layer/EditablePolygonItem.h
    layer/TransformHandleItem.h
    layer/MaskLayer.h
    layer/MaskLayerItem.h
    undo/AbstractCommand.h
    undo/CageMoveCommand.h
    undo/CageWarpCommand.h
    undo/PaintCommand.h
    undo/PaintStrokeCommand.h
    undo/InvertLayerCommand.h
    undo/MaskStrokeCommand.h
    undo/MaskPaintCommand.h
    undo/MirrorLayerCommand.h
    undo/TransformLayerCommand.h
    util/GeometryUtils.h
)

# Qt6 Executable
qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
