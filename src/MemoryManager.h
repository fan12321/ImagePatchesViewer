#pragma once

#include "ImagePatchesViewer.h"
#include <map>
#include <set>
#include <vector>
#include <QImage>
#include <QString>
#include <QDebug>

class ImagePatchesViewer;

class MemoryManager {
public:
    MemoryManager(ImagePatchesViewer*);
    ~MemoryManager();

    std::map<int, QString>  indexFilenameMap;

    void setImageDir(QString path) { _imageDir = path; };
    void setMaxImagesInCache(int n) { _cacheSize = n; };

    void unloadImages(std::vector<unsigned int>&);
    void loadImages(std::vector<unsigned int>&);

    int findImageToDrop();

    QImage* getImage(int index) { return pointer[index]; };

private:
    ImagePatchesViewer*                     _plugin;
    std::map<unsigned int, QImage*>         pointer;
    std::map<unsigned int, int>             cache;
    std::map<unsigned int, int>             count;
    
    enum LoadingStatus {
        NOTLOADED = 0, 
        LOADING = 1, 
        LOADED = 2
    };
    std::map<unsigned int, LoadingStatus>   status;
    QString                                 _imageDir;
    int                                     _cacheSize;
};