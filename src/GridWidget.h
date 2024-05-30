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
        _transform.reset(); 
        resize(_parent->size().width(), _parent->size().height()); 
    }
    void paintEvent(QPaintEvent*) override;
    void paint(int, QPainter*, QPen*);

    // update currently focused grid
    void setIndices(std::vector<unsigned int>);
    std::vector<unsigned int> getIndices();
    mv::Dataset<Points> _points;

private:
    bool eventFilter(QObject*, QEvent*);
    void changeGrid(int);

    MemoryManager*  _mm;

    QString         _imageDir;
    QWidget*        _parent;
    QTransform      _transform;

    int   _maxImagesInCache;
    
    float _imgWidth;
    float _imgHeight;
    float _ratio;

    int _gridCount;
    int _currentGridId;
    std::vector<Grid*> _grids;

    QImage emptyImage = QImage(_imgWidth, _imgHeight, QImage::Format_ARGB32);

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;



public slots:
    void ShowContextMenu(const QPoint&);
    void newSelection();
    void deleteSelection();
};