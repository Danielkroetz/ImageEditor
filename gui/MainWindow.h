#pragma once

#include <QCoreApplication>
#include <QComboBox>
#include <QMainWindow>
#include <QString>
#include <QUndoView>
#include <QListWidget>
#include <QSpinBox>
#include <QToolBar>

class ImageView;
class LayerItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:

    enum MainOperationMode { Paint, Mask, FreeSelection, Polygon, ImageLayer, CreateLasso, CreatePolygon };

    explicit MainWindow( const QString& imagePath = QString(), const QString& historyPath = QString(), 
                bool useVulkan = false, QWidget* parent = nullptr );
    
    // ----------------- global setter and getter -----------------
    ImageView* getViewer() const { return m_imageView; }
    int getNumberOfCageControlPoints() const { return m_cageControlPointsSpin->value(); }
    void setMainOperationMode( MainOperationMode = ImageLayer );

private slots:

    void rebuildLayerList();
    void newLassoLayerCreated();
    void toggleLayerVisibility( QListWidgetItem* item );
    void layerItemClicked( QListWidgetItem* item );
    void onLayerItemClicked( QListWidgetItem* item );
    void showLayerContextMenu( const QPoint& pos );
    
    void deleteLayer();
    void duplicateLayer();
    void renameLayer();
    
    void zoom1to1();
    void fitToWindow();
    void openImage();
    void saveAsImage();
    void openHistory();
    void saveHistory();
    void cutSelection();
    void toggleDocks();
    void createMaskImage();
    void forcedUpdate();
    void info();
    
    void updateButtonState();
    void updateControlButtonState();

private:

    bool loadImage( const QString& );
    void loadHistory( const QString& );
    bool saveProject( const QString& );
    bool loadProject( const QString&, bool );
    
    void createDockWidgets();
    void createActions();
    void createToolbars();
    void createStatusbar();
    
    QComboBox* buildDefaultColorComboBox( const QString& name = "Label" );
    
    ImageView* m_imageView = nullptr;
    LayerItem* m_layerItem = nullptr;
    
    QUndoView* m_undoView;
    QDockWidget* m_layerDock;
    QDockWidget* m_historyDock;
    QListWidget* m_layerList;
    
    QToolBar* m_editToolbar = nullptr;
    QToolBar* m_lassoToolbar = nullptr;
    QToolBar* m_layerToolbar = nullptr;
    QToolBar* m_maskToolbar = nullptr;
    QToolBar* m_polygonToolbar = nullptr;
    
    QSpinBox* m_cageControlPointsSpin = nullptr;
    
    QAction* m_saveHistoryAction = nullptr;
    QAction* m_openHistoryAction = nullptr;
    QAction* m_createMaskImageAction = nullptr;
    QAction* m_openMaskImageAction = nullptr;
    QAction* m_saveMaskImageAction = nullptr;
    QAction* m_paintMaskImageAction = nullptr;
    QAction* m_eraseMaskImageAction = nullptr;
    QAction* m_paintControlAction = nullptr;
    QAction* m_lassoControlAction = nullptr;
    QAction* m_maskControlAction = nullptr;
    QAction* m_layerControlAction = nullptr;
    QAction* m_polygonControlAction = nullptr;
    QAction* m_openAction = nullptr;
    QAction* m_saveAsAction = nullptr;
    QAction* m_cutAction = nullptr;
    QAction* m_pipetteAction = nullptr;
    QAction* m_zoom1to1Action = nullptr;
    QAction* m_fitAction = nullptr;
    QAction* m_crosshairAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;
    QAction* m_paintAction = nullptr;
    QAction* m_showDockWidgets = nullptr;
    QAction* m_lassoAction = nullptr;
    QAction* m_polygonAction = nullptr;
    QAction* m_infoAction = nullptr; 
    
    bool m_updatingLayerList = false;
    bool m_saveImageDataInProjectFile = false;
    
};

