#pragma once

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

#include <map>
#include <set>
#include <future>

#include "MemoryManager.h"

class GridWidget : public QWidget
{
    Q_OBJECT
public:
    GridWidget(QWidget*, QString, MemoryManager*);

    void unloadImages(std::set<int>&);
    void loadImages(std::set<int>&);

    void resetView() { _scale = 1.0; _transform.reset(); resize(_parent->size().width(), _parent->size().height()); }
    void paintEvent(QPaintEvent*) override;

private:
    bool eventFilter(QObject*, QEvent*);

    MemoryManager*              _mm;

    QString                     _imageDir;
    QWidget*                    _parent;
    QTransform                  _transform;

    float       _scale = 1.0;
    int         _size = 128;
    int         _x = 0;
    int         _y = 0;
    int         _originX = 0;
    int         _originY = 0;
    int         _maxImagesInCache;
    
    float _imgWidth;
    float _imgHeight;
    float _ratio;

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
};