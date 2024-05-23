#include "MemoryManager.h"

MemoryManager::MemoryManager()
{

}

MemoryManager::~MemoryManager()
{
    for (auto pair: table) delete pair.second;
    for (auto pair: cache) delete pair.second;
}

void MemoryManager::unLoadImage(int index) {
    if (cache.size() < _cacheSize) {
        cache[index] = table[index];
        table.erase(index);
    }
    else {
        delete table[index];
    }
    indices.erase(index);
}

void MemoryManager::loadImage(int index) {
    if (cache.count(index)) {
        table[index] = cache[index];
        indices.insert(index);
    }
    else {
        QString path = _imageDir + "/" + indexFilenameMap[index] + ".jpg";
        QImage* img = new QImage(path);
        table[index] = img;
        indices.insert(index);
    }
}
