#include "MemoryManager.h"
#include <QtConcurrent>
#include <QFuture>

MemoryManager::MemoryManager()
{

}

MemoryManager::~MemoryManager()
{
    for (auto pair: table) delete pair.second;
    for (auto pair: cache) delete pair.second;
}

int MemoryManager::findImageToDrop(std::set<int>& loads) {
    int toDrop = -1;
    for (auto pair: cache) {
        if (!loads.count(pair.first)) {
            toDrop = pair.first;
            break;
        }
    }
    return toDrop;
}

void MemoryManager::unloadImages(std::set<int>& unloads, std::set<int>& loads) {
    pointersToRemove.clear();
    for (int index: unloads) {
        // cache still available, store in cache
        if (cache.size() < _cacheSize && !cache[index]) {
            cache[index] = table[index];
        }
        else {
            // find a spot in cache that will not soon be loaded
            int toDrop = findImageToDrop(loads);
            if (toDrop > -1) {
                pointersToRemove.push_back(cache[toDrop]);
                cache.erase(toDrop);
                cache[index] = table[index];
            }
            else {
                pointersToRemove.push_back(table[toDrop]);
            }
        }
        table.erase(index);
        indices.erase(index);
    }
}

void MemoryManager::loadImages(std::set<int>& s) {
    for (int index: s) {
        if (cache[index]) {
            table[index] = cache[index];
            cache.erase(index);
            indices.insert(index);
        }
        else {
            indices.insert(index);
            QString path = _imageDir + "/" + indexFilenameMap[index] + ".jpg";

            // QImage* img = new QImage(path);
            // table[index] = img;

            QtConcurrent::task([this](QString p, int idx){
                QImage* img = new QImage(p);
                table[idx] = img;
            })
            .withArguments(path, index)
            .spawn()
            .then([this](){
                _plugin->getGridWidget()->update();
            });

        }
    }
}
