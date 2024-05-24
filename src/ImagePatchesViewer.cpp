#include "ImagePatchesViewer.h"

#include <event/Event.h>
#include <DatasetsMimeData.h>
#include <QDebug>
#include <QMimeData>
#include <QFileDialog>


Q_PLUGIN_METADATA(IID "nl.BioVault.ImagePatchesViewer")

using namespace mv;

ImagePatchesViewer::ImagePatchesViewer(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _points(),
    _clusters(), 
    _currentDatasetName(),
    _currentDatasetNameLabel(new QLabel()), 
    _imageDir(""), 
    _validPath(false),
    _grid(nullptr),
    _mm(new MemoryManager(this))
{
    // This line is mandatory if drag and drop behavior is required
    _currentDatasetNameLabel->setAcceptDrops(true);

    // Align text in the center
    _currentDatasetNameLabel->setAlignment(Qt::AlignCenter);
}

void ImagePatchesViewer::init()
{
    // Create layout
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(_currentDatasetNameLabel);
    layout->addWidget(_grid, 1);

    // Apply the layout
    getWidget().setLayout(layout);

    // Instantiate new drop widget
    _dropWidget = new DropWidget(_currentDatasetNameLabel);

    // Set the drop indicator widget (the widget that indicates that the view is eligible for data dropping)
    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));

    // Initialize the drop regions
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        // A drop widget can contain zero or more drop regions
        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        // Gather information to generate appropriate drop regions
        const auto dataset = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName = dataset->getGuiName();
        const auto datasetId = dataset->getId();
        const auto dataType = dataset->getDataType();
        const auto dataTypes = DataTypes({ ClusterType });

        // Visually indicate if the dataset is of the wrong data type and thus cannot be dropped
        if (!dataTypes.contains(dataType)) {
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);
        }
        else {
            // Get points dataset from the core
            auto candidateDataset = mv::data().getDataset<Clusters>(datasetId);

            // Accept points datasets drag and drop
            if (dataType == ClusterType) {
                const auto description = QString("Load %1 into image patches view").arg(datasetGuiName);

                if (_clusters == candidateDataset) {
                    // Dataset cannot be dropped because it is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", "exclamation-circle", false);
                }
                else {
                    // Dataset can be dropped
                    dropRegions << new DropWidget::DropRegion(this, "Clusters", description, "map-marker-alt", true, [this, candidateDataset]() {
                        imageDirInquire(candidateDataset);
                    });
                }
            }
        }

        return dropRegions;
    });

    // Respond when the name of the dataset in the dataset reference changes
    connect(&_clusters, &Dataset<Points>::guiNameChanged, this, [this]() {

        auto newDatasetName = _clusters->getGuiName();

        // Update the current dataset name label
        _currentDatasetNameLabel->setText(QString("Current clusters dataset: %1").arg(newDatasetName));

        // Only show the drop indicator when nothing is loaded in the dataset reference
        _dropWidget->setShowDropIndicator(newDatasetName.isEmpty());
    });

    // Alternatively, classes which derive from hdsp::EventListener (all plugins do) can also respond to events
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&ImagePatchesViewer::onDataEvent, this, std::placeholders::_1));
}

void ImagePatchesViewer::imageDirInquire(mv::Dataset<Clusters> candidateDataset) {
    _clusters = candidateDataset;

    
    // TODO: change this!!!
    // _imageDir = QString("/home/chen-chi/Desktop/ManiVault/projects/images");
    // _validPath = true;

    _imageDir = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Image folder"),
        _imageDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    _validPath = !_imageDir.isNull();


    if (_validPath) {

        auto clusters = _clusters->getClusters();
        for (auto cluster: clusters) {
            int index = cluster.getIndices()[0];
            QString fileName = cluster.getName();
            _mm->indexFilenameMap[index] = fileName;
        }
        _mm->setImageDir(_imageDir);
        _grid = new GridWidget(&getWidget(), _imageDir, _mm);
        _dropWidget->setShowDropIndicator(false);
    }
}

void ImagePatchesViewer::onDataEvent(mv::DatasetEvent* dataEvent)
{
    // Get smart pointer to dataset that changed
    auto changedDataSet = dataEvent->getDataset();

    // Get GUI name of the dataset that changed
    const auto datasetGuiName = changedDataSet->getGuiName();

    // The data event has a type so that we know what type of data event occurred (e.g. data added, changed, removed, renamed, selection changes)
    if (dataEvent->getType() == EventType::DatasetDataSelectionChanged) {
        // Cast the data event to a data selection changed event
        const auto dataSelectionChangedEvent = static_cast<DatasetDataSelectionChangedEvent*>(dataEvent);

        // Get the selection set that changed
        const auto& selectionSet = changedDataSet->getSelection<Points>();


        // TODO: change the condition!!
        if ( _validPath && 1
                // _clusters->getGroupIndex() == changedDataSet->getGroupIndex() && 
                // _clusters->getGroupIndex() != -1 
            ) {
            int selectionSize = changedDataSet->getSelectionSize();

            if (selectionSize == 0) return;

            auto selectionIndices = changedDataSet->getSelectionIndices();
            std::set<int> toLoad;
            std::set<int> toKeep;
            std::set<int> toUnload;
            for (int index: selectionIndices) {
                if (!_mm->indices.count(index)) {
                    toLoad.insert(index);
                }
                else {
                    toKeep.insert(index);
                }
            }
            for (int index: _mm->indices) {
                if (!toKeep.count(index)) {
                    toUnload.insert(index);
                }
            }

            _mm->loadImages(toLoad);
            _mm->unloadImages(toUnload, toLoad);
            
            if (toUnload.size() == 0 && toLoad.size() == 0) return;
            else {
                _grid->resetView();
                _grid->update();
                // postpone the "free memory" action to prevent seg fault
                _mm->deleteImages();
            }

        }
    }
}


ViewPlugin* ImagePatchesViewerFactory::produce()
{
    return new ImagePatchesViewer(this);
}

mv::DataTypes ImagePatchesViewerFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(ClusterType);

    return supportedTypes;
}

mv::gui::PluginTriggerActions ImagePatchesViewerFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this]() -> ImagePatchesViewer* {
        return dynamic_cast<ImagePatchesViewer*>(plugins().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (numberOfDatasets == 1 && PluginFactory::areAllDatasetsOfTheSameType(datasets, ClusterType)) {
        auto pluginTriggerAction = new PluginTriggerAction(const_cast<ImagePatchesViewerFactory*>(this), this, "Image Patches Viewer", "Image grid", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
            for (auto dataset : datasets)
                getPluginInstance()->imageDirInquire(dataset);
        });
        pluginTriggerActions << pluginTriggerAction;
    }

    return pluginTriggerActions;
}
