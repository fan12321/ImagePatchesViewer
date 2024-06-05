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

void Grid::insertAfter(Grid* previous) {
    this->_next = previous->_next;
    this->_previous = previous;
    previous->_next = this;
    this->_next->_previous = this;
}

void Grid::removeFromLinkedList() {
    this->_previous->_next = this->_next;
    this->_next->_previous = this->_previous;
}