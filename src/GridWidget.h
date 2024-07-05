#pragma once

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>

#include <map>
#include <vector>
#include <future>

#include "MemoryManager.h"
#include "Grid.h"

#define MAX_SELECTION 3

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

class MemoryManager;

class GridWidget : public QWidget
{
    Q_OBJECT
public:
    GridWidget(QWidget*, QString, MemoryManager*, mv::Dataset<Points>);
    ~GridWidget();

    void resetView() {
        _currentGrid->_keepLayout = false;
        _transform.reset(); 
        resize(_parent->size().width(), _parent->size().height()); 
    }
    void paintEvent(QPaintEvent*) override;
    void paint(Grid*, QPainter*, QPen*);

    // update currently focused grid
    void setIndices(std::vector<unsigned int>);
    std::vector<unsigned int> getIndices();

private:
    bool eventFilter(QObject*, QEvent*);
    void changeGrid(Grid*);

    MemoryManager*  _mm;
    mv::Dataset<Points> _points;

    QString         _imageDir;
    QWidget*        _parent;
    QTransform      _transform;

    int   _maxImagesInCache;
    
    float _imgWidth;
    float _imgHeight;
    float _ratio;

    int _gridCount;
    Grid* _currentGrid;

    QImage emptyImage = QImage(_imgWidth, _imgHeight, QImage::Format_ARGB32);

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;



public slots:
    void ShowContextMenu(const QPoint&);
    void newSelection();
    void deleteSelection();
    void toggleLayout();
    void rearrange();
};