#pragma once
#include <QWidget>
#include <vector>

class Grid {
public:
    Grid(int);
    ~Grid() {};
    bool inside(QPoint);
    void insertAfter(Grid*);
    void removeFromLinkedList();

    std::vector<unsigned int> indices;

    Grid*       _next = this;
    Grid*       _previous = this;
    float       _ratio = 0.0;
    float       _scale = 1.0;
    int         _x = 0;
    int         _y = 0;
    int         _originX = 0;
    int         _originY = 0;
    QTransform  _transform;
    int         _size;

    bool        _keepLayout = false;
    int         _leftOffset, _topOffset;
};