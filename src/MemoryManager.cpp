#include "MemoryManager.h"
#include <QtConcurrent>
#include <QFuture>

MemoryManager::MemoryManager(ImagePatchesViewer* p) :
    _plugin(p)
{
    _start = time(nullptr);
}

MemoryManager::~MemoryManager()
{
    for (auto pair: pointer) delete pair.second;
}

int MemoryManager::findImageToDrop() {
    int toDrop = -1;
    double min_ = std::numeric_limits<double>::infinity();
    for (auto pair: cache) {
        int index = pair.first;
        int priority = pair.second;
        if (pointer[index] == nullptr) continue;
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
            // QString path = indexFilenameMap[index];
            QString path = filenames[index];
            status[index] = LOADING;
            if (_imagesOnScreen > _maxImagesOnScreen) {
                postponeLoadIndices.insert(index);
            }
            else {
                _imagesOnScreen += 1;
                QtConcurrent::task([this](QString p, int idx){
                    QImage* img = new QImage(p);

                    if (pointer[idx] || status[idx] == NOTLOADED) {
                        delete img;
                    }
                    else {
                        pointer[idx] = img;
                        status[idx] = LOADED;
                    }
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
            // already in cache
            else if (cache.count(index)) {
                cache[index] = difftime(time(nullptr), _start);
                _imagesOnScreen -= 1;
            }
            // cache still availble
            else if (_cacheSize < _maxCacheSize) {
                cache[index] = difftime(time(nullptr), _start);
                _imagesOnScreen -= 1;
                _cacheSize += 1;
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
                    cache[index] = difftime(time(nullptr), _start);
                }
                _imagesOnScreen -= 1;
            }
        }
        count[index] -= 1;
    }
    postponeLoad();
}

void MemoryManager::postponeLoad() {
    auto it = postponeLoadIndices.begin();
    while (it != postponeLoadIndices.end() && _imagesOnScreen <= _maxImagesOnScreen) {
        if (_imagesOnScreen == _maxImagesOnScreen) {
            int toDrop = findImageToDrop();
            delete pointer[toDrop];
            pointer[toDrop] = nullptr;
            status[toDrop] = NOTLOADED;
            cache.erase(toDrop);
        }
        int index = *it;
        it = postponeLoadIndices.erase(it);
        _imagesOnScreen += 1;

        // QString path = indexFilenameMap[index];
        QString path = filenames[index];
        status[index] = LOADING;
        QtConcurrent::task([this](QString p, int idx){
            QImage* img = new QImage(p);

            if (pointer[idx] || status[idx] == NOTLOADED) {
                delete img;
            }
            else {
                pointer[idx] = img;
                status[idx] = LOADED;
            }
        })
        .withArguments(path, index)
        .spawn()
        .then([this](){
            _plugin->getGridWidget()->update();
        });
    }
}