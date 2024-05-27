#pragma once

#include <ViewPlugin.h>
#include <Dataset.h>
#include <widgets/DropWidget.h>

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>

#include <QResizeEvent>
#include <QWidget>
#include <QImage>
#include <set>
#include "GridWidget.h"
#include "MemoryManager.h"

/** All plugin related classes are in the ManiVault plugin namespace */
using namespace mv::plugin;

/** Drop widget used in this plugin is located in the ManiVault gui namespace */
using namespace mv::gui;

/** Dataset reference used in this plugin is located in the ManiVault util namespace */
using namespace mv::util;

class GridWidget;
class MemoryManager;

class ImagePatchesViewer : public ViewPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    ImagePatchesViewer(const PluginFactory* factory);

    /** Destructor */
    ~ImagePatchesViewer() override = default;
    
    /** This function is called by the core after the view plugin has been created */
    void init() override;

    /**
     * Invoked when a data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(mv::DatasetEvent* dataEvent);

    void imageDirInquire(mv::Dataset<Clusters>);

    GridWidget* getGridWidget() { return _gridWidget; };

protected:
    DropWidget*             _dropWidget;                /** Widget for drag and drop behavior */
    mv::Dataset<Points>     _points;                    /** Points smart pointer */
    mv::Dataset<Clusters>   _clusters;
    QString                 _currentDatasetName;        /** Name of the current dataset */
    QString                 _imageDir;                  /** Path to the images */
    bool                    _validPath;                 /** Check if "_imageDir" valid or not */
    QLabel*                 _currentDatasetNameLabel;   /** Label that show the current dataset name */
    MemoryManager*          _mm;
    GridWidget*             _gridWidget;
};

/**
 * Example view plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
class ImagePatchesViewerFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.BioVault.ImagePatchesViewer"
                      FILE  "ImagePatchesViewer.json")

public:

    /** Default constructor */
    ImagePatchesViewerFactory() {}

    /** Destructor */
    ~ImagePatchesViewerFactory() override {}
    
    /** Creates an instance of the example view plugin */
    ViewPlugin* produce() override;

    /** Returns the data types that are supported by the example view plugin */
    mv::DataTypes supportedDataTypes() const override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
