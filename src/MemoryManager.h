#pragma once

#include <map>
#include <set>
#include <QImage>
#include <QString>

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    std::set<int>           indices;
    std::map<int, QString>  indexFilenameMap;

    void setImageDir(QString path) { _imageDir = path; };
    void setMaxImagesInCache(int n) { _cacheSize = n; };

    void unLoadImage(int);
    void loadImage(int);

    QImage* getImage(int index) { return table[index]; };

private:
    std::map<int, QImage*>  table;
    std::map<int, QImage*>  cache;
    QString                 _imageDir;
    int                     _cacheSize;
};