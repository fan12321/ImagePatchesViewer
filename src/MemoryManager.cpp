#include "MemoryManager.h"
#include <QtConcurrent>
#include <QFuture>

MemoryManager::MemoryManager(ImagePatchesViewer* p) :
    _plugin(p)
{

}

MemoryManager::~MemoryManager()
{
    for (auto pair: pointer) delete pair.second;
}

int MemoryManager::findImageToDrop() {
    int toDrop = -1;
    int min_ = 101;
    for (auto pair: cache) {
        int index = pair.first;
        int priority = pair.second;
        if (priority < min_) {
            min_ = priority;
            toDrop = index;
        }
    }
    return toDrop;
}

void MemoryManager::loadImages(std::vector<unsigned int>& toLoad) {
    for (auto index: toLoad) {
        if (pointer[index] != nullptr) {
            
        }
        else {
            QString path = _imageDir + "/" + indexFilenameMap[index] + ".jpg";
            QtConcurrent::task([this](QString p, int idx){
                QImage* img = new QImage(p);
                pointer[idx] = img;
            })
            .withArguments(path, index)
            .spawn()
            .then([this](){
                _plugin->getGridWidget()->update();
            });

        }
        count[index] += 1;
    }
}

void MemoryManager::unloadImages(std::vector<unsigned int>& toUnload) {
    for (auto index: toUnload) {
        // other grids still need it
        if (count[index] > 1) {

        }
        // image not needed anymore
        else if (count[index] == 1) {
            // cache still availble
            if (cache.size() < _cacheSize || cache[index] > 0) {
                cache[index] = std::min(cache[index], 100);
            }
            // cache full, find one to drop
            else {
                int toDrop = findImageToDrop();
                delete pointer[toDrop];
                pointer[toDrop] = nullptr;
                cache.erase(toDrop);
                cache[index] = 1;
            }
        }
        count[index] -= 1;
    }
}