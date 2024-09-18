#pragma once

#include "ImagePatchesViewer.h"
#include <map>
#include <set>
#include <vector>
#include <QImage>
#include <QString>
#include <QDebug>
#include <time.h>
#include<limits>

class ImagePatchesViewer;

class MemoryManager {
public:
    MemoryManager(ImagePatchesViewer*);
    ~MemoryManager();

    std::map<int, QString>  indexFilenameMap;
    std::vector<QString> filenames;

    void setMaxCacheSize(int n) { _maxCacheSize = n; };
    void setMaxImagesLoaded(int n) { _maxImagesOnScreen = n; };

    void unloadImages(std::vector<unsigned int>&);
    void loadImages(std::vector<unsigned int>&);
    void postponeLoad();

    int findImageToDrop();

    QImage* getImage(int index) { return pointer[index]; };

private:
    int _maxImagesOnScreen = 300;
    int _imagesOnScreen = 0;
    std::set<unsigned int> postponeLoadIndices;

    ImagePatchesViewer*                     _plugin;
    std::map<unsigned int, QImage*>         pointer;
    std::map<unsigned int, double>          cache;      /* stores the priority value */
    std::map<unsigned int, int>             count;      /* stores the number of grids that are using the image */
    time_t _start;

    enum LoadingStatus {
        NOTLOADED = 0, 
        LOADING = 1, 
        LOADED = 2
    };
    std::map<unsigned int, LoadingStatus>   status;
    unsigned int                            _maxCacheSize;
    unsigned int                            _cacheSize;
};