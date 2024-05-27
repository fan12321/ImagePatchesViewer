#pragma once

#include "ImagePatchesViewer.h"
#include <map>
#include <set>
#include <vector>
#include <QImage>
#include <QString>

class ImagePatchesViewer;

class MemoryManager {
public:
    MemoryManager(ImagePatchesViewer*);
    ~MemoryManager();

    std::set<int>           indices;
    std::map<int, QString>  indexFilenameMap;

    void setImageDir(QString path) { _imageDir = path; };
    void setMaxImagesInCache(int n) { _cacheSize = n; };

    void unloadImages(std::set<int>&, std::set<int>&);
    void loadImages(std::set<int>&);
    int findImageToDrop(std::set<int>&);
    void deleteImages() {
        for (auto it=pointersToRemove.begin(); it!=pointersToRemove.end(); it++) {
            delete *it;
        }    
    };

    QImage* getImage(int index) { return table[index]; };

private:
    ImagePatchesViewer*     _plugin;
    std::map<int, QImage*>  table;
    std::map<int, QImage*>  cache;
    std::vector<QImage*>    pointersToRemove;
    QString                 _imageDir;
    int                     _cacheSize;
};