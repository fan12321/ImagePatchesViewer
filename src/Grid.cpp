#include "Grid.h"

Grid::Grid(int w) :
    _size(w)
{
    _transform.reset();
}

bool Grid::inside(QPoint windowSpacePoint) {
    QPoint gridSpacePoint = _transform.inverted().map(windowSpacePoint);
    return (gridSpacePoint.x() > 0 &&
        gridSpacePoint.y() > 0 &&
        gridSpacePoint.x() < _size && 
        gridSpacePoint.y() < _size * _ratio
    );
}