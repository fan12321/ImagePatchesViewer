#pragma once

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

#include <map>
#include <set>
#include <vector>
#include <future>

#include "MemoryManager.h"
#include "Grid.h"

class MemoryManager;

class GridWidget : public QWidget
{
    Q_OBJECT
public:
    GridWidget(QWidget*, QString, MemoryManager*);
    ~GridWidget() { delete _mm; };

    void resetView() { 
        _scale = 1.0; 
        _transform.reset(); 
        resize(_parent->size().width(), _parent->size().height()); 
    }
    void paintEvent(QPaintEvent*) override;

private:
    bool eventFilter(QObject*, QEvent*);

    MemoryManager*              _mm;

    QString                     _imageDir;
    QWidget*                    _parent;
    QTransform                  _transform;

    float       _scale = 1.0;
    int         _x = 0;
    int         _y = 0;
    int         _originX = 0;
    int         _originY = 0;
    int         _maxImagesInCache;
    
    float _imgWidth;
    float _imgHeight;
    float _ratio;

    int _focusGrid;
    std::vector<Grid*> _grids;

    QImage emptyImage = QImage(_imgWidth, _imgHeight, QImage::Format_ARGB32);

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
};