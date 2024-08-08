#pragma once

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>

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
        _currentGrid->_originPosition = false;
        action3->setChecked(false);
        _transform.reset(); 
        resize(_parent->size().width(), _parent->size().height()); 
    }
    void paintEvent(QPaintEvent*) override;

    // update currently focused grid
    void setIndices(std::vector<unsigned int>);
    std::vector<unsigned int> getIndices();

private:
    bool eventFilter(QObject*, QEvent*);
    void changeGrid(Grid*);

    void paint(Grid*, QPainter*, QPen*);

    MemoryManager*  _mm;
    mv::Dataset<Points> _points;

    QString         _imageDir;
    QWidget*        _parent;
    QTransform      _transform;

    QAction* action1;
    QAction* action2;
    QAction* action3;
    
    float _imgWidth;
    float _imgHeight;
    float _ratio;

    int _gridCount;
    Grid* _currentGrid;

    QImage emptyImage = QImage(_imgWidth, _imgHeight, QImage::Format_ARGB32);

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;


public slots:
    void ShowContextMenu(const QPoint&);
    void newSelection();
    void deleteSelection();
    void toggleLayout();
    void rearrange();
};