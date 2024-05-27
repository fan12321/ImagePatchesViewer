#include "Grid.h"

Grid::Grid(int w) :
    size(w)
{
    _transform.reset();
}

bool Grid::inside(QPoint windowSpacePoint) {
    QPoint gridSpacePoint = _transform.inverted().map(windowSpacePoint);
    return (gridSpacePoint.x() > 0 &&
        gridSpacePoint.y() > 0 &&
        gridSpacePoint.x() < size && 
        gridSpacePoint.y() < size * _ratio
    );
}