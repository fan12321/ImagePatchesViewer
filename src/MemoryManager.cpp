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
        if (status[index] != LOADED) continue;
        else if (priority < min_) {
            min_ = priority;
            toDrop = index;
        }
    }
    return toDrop;
}

void MemoryManager::loadImages(std::vector<unsigned int>& toLoad) {
    for (auto index: toLoad) {
        if (pointer[index] == nullptr && status[index] == NOTLOADED) {
            QString path = _imageDir + "/" + indexFilenameMap[index] + ".jpg";
            status[index] = LOADING;
            if (_totalLoaded > _maxImagesLoaded) {
                postponeLoadIndices.insert(index);
            }
            else {
                _totalLoaded += 1;
                QtConcurrent::task([this](QString p, int idx){
                    QImage* img = new QImage(p);
                    pointer[idx] = img;
                    status[idx] = LOADED;
                })
                .withArguments(path, index)
                .spawn()
                .then([this](){
                    _plugin->getGridWidget()->update();
                });
            }
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
            if (postponeLoadIndices.count(index)) {
                postponeLoadIndices.erase(index);
                status[index] = NOTLOADED;
            }
            // cache still availble or already in cache
            else if (cache.size() < _cacheSize || cache.count(index)) {
                cache[index] = std::min(cache[index]+1, 100);
                _totalLoaded -= 1;
            }
            // cache full, find one to drop
            else {
                int toDrop = findImageToDrop();
                if (toDrop == -1) {
                    delete pointer[index];
                    pointer[index] = nullptr;
                    status[index] = NOTLOADED;
                }
                else {
                    delete pointer[toDrop];
                    pointer[toDrop] = nullptr;
                    status[toDrop] = NOTLOADED;
                    cache.erase(toDrop);
                    cache[index] = 1;
                }
                _totalLoaded -= 1;
            }
        }
        count[index] -= 1;
    }
    postponeLoad();
}

void MemoryManager::postponeLoad() {
    auto it = postponeLoadIndices.begin();
    while (it != postponeLoadIndices.end() && _totalLoaded <= _maxImagesLoaded) {
        if (_totalLoaded == _maxImagesLoaded) {
            int toDrop = findImageToDrop();
            delete pointer[toDrop];
            pointer[toDrop] = nullptr;
            status[toDrop] = NOTLOADED;
            cache.erase(toDrop);
        }
        int index = *it;
        it = postponeLoadIndices.erase(it);
        _totalLoaded += 1;

        QString path = _imageDir + "/" + indexFilenameMap[index] + ".jpg";
        status[index] = LOADING;
        QtConcurrent::task([this](QString p, int idx){
            QImage* img = new QImage(p);
            pointer[idx] = img;
            status[idx] = LOADED;
        })
        .withArguments(path, index)
        .spawn()
        .then([this](){
            _plugin->getGridWidget()->update();
        });
    }
}