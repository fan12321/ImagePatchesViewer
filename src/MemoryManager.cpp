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

void MemoryManager::loadImages(std::vector<unsigned int>& toLoad) {
    for (auto index: toLoad) {
        if (count[index] > 0) {
            
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
        if (count[index] > 1) {
            count[index] -= 1;
        }
        else if (count[index] == 1) {
            count[index] = 0;
            delete pointer[index];
            pointer[index] = nullptr;
        }
    }
}