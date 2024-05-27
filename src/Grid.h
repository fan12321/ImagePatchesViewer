#pragma once
#include <QWidget>

class Grid {
public:
    Grid(int);
    bool inside(QPoint);

    float       _ratio = 1.0;
    float       _scale = 1.0;
    int         _x = 0;
    int         _y = 0;
    int         _originX = 0;
    int         _originY = 0;
    QTransform  _transform;

private:
    int size;
};