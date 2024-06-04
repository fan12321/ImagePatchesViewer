#include "GridWidget.h"
#include <QImage>
#include <cmath>

GridWidget::GridWidget(QWidget* parent, QString imageDir, MemoryManager* mm, mv::Dataset<Points> points) : 
    QWidget(parent),
    _parent(parent),
    _imageDir(imageDir),
    _mm(mm),
    _points(points)
{

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), 
        this, SLOT(ShowContextMenu(const QPoint&)));

    if (!parent) return;
    installEventFilter(this);
    resize(parent->size().width(), parent->size().height());

    QImage* img = new QImage(_imageDir + "/" + _mm->indexFilenameMap[0] + ".jpg");
    _imgWidth = img->size().width();
    _imgHeight = img->size().height();
    _ratio = _imgWidth / _imgHeight;

    // 5GB of images in cache, wierd calculation to avoid overflow
    _maxImagesInCache = (int) ceil(5.0 * 1024.0 * (1024.0 / _imgWidth) * (1024.0 / _imgHeight) * (1.0 / sizeof(*img)));
    delete img;

    _mm->setMaxImagesInCache(50);
    // _mm->setMaxImagesInCache(_maxImagesInCache);

    emptyImage.fill(QColor(0, 0, 0, 0));

    _grids[0] = new Grid(parent->size().width());
    _currentGridId = 0;
    _gridCount = 1;

    show();
};

void GridWidget::setIndices(std::vector<unsigned int> indices) {
    _grids[_currentGridId]->indices = indices;
}

std::vector<unsigned int> GridWidget::getIndices() {
    return _grids[_currentGridId]->indices;
}

void GridWidget::ShowContextMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("New selection", this);
    connect(&action1, SIGNAL(triggered()), this, SLOT(newSelection()));
    QAction action2("Delete selection", this);
    connect(&action2, SIGNAL(triggered()), this, SLOT(deleteSelection()));

    action1.setEnabled(_gridCount < MAX_SELECTION);
    action2.setEnabled(_gridCount != 1);

    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);


    contextMenu.exec(mapToGlobal(pos));
}

void GridWidget::changeGrid(int gridIndex) {
    Grid* grid = _grids[gridIndex];
    _points->setSelectionIndices(grid->indices);
    mv::events().notifyDatasetDataSelectionChanged(_points);
}

void GridWidget::newSelection() {
    if (_grids[_currentGridId]->indices.size() == 0) {
        return;
    }
    _grids[_gridCount] = new Grid(_parent->size().width());
    _currentGridId = _gridCount;
    _gridCount += 1;
    changeGrid(_currentGridId);
}

void GridWidget::deleteSelection() {
    _mm->unloadImages(_grids[_currentGridId]->indices);
    delete _grids[_currentGridId];
    for (int i=_currentGridId+1; i<_gridCount; i++) {
        _grids[i-1] = _grids[i];
    }
    _gridCount -= 1;
    _currentGridId = (_currentGridId - 1 + _gridCount) % _gridCount;
    changeGrid(_currentGridId);
}


void GridWidget::wheelEvent(QWheelEvent* event)
{
    auto scroll = event->angleDelta().y();
    if (scroll != 0) {
        float scale = (event->angleDelta().y() > 0? 1.1: 0.91);
        Grid* grid = _grids[_currentGridId];

        grid->_scale *= scale;

        QPoint transformedMousePosition = grid->_transform.inverted().map(mapFromGlobal(QCursor::pos()));
        grid->_x = transformedMousePosition.x();
        grid->_y = transformedMousePosition.y();

        // column major
        QTransform t1(
            1, 0, 0,
            0, 1, 0, 
            grid->_x, grid->_y, 1    
        );
        QTransform s(
            scale, 0, 0, 
            0, scale, 0, 
            0, 0, 1
        );
        QTransform t2(
            1, 0, 0,
            0, 1, 0, 
            -grid->_x, -grid->_y, 1    
        );
        grid->_transform = t2 * s * t1 * grid->_transform;
        update();
    }
}

void GridWidget::mousePressEvent(QMouseEvent* event) {
    Grid* grid = _grids[_currentGridId];
    int previousGridId = _currentGridId;
    int previousGridSize = _grids[previousGridId]->indices.size();
    if (!grid->inside(event->pos()) || previousGridSize == 0) {
        for (int i=0; i<_gridCount; i++) {
            grid = _grids[i];
            if (grid->inside(event->pos())) {
                if (previousGridSize == 0) {
                    for (int j=previousGridId+1; j<_gridCount; j++) {
                        _grids[j-1] = _grids[j];
                    }
                    _gridCount -= 1;
                    _currentGridId = (i > previousGridId)? i-1 : i;
                }
                else {
                    _currentGridId = i;
                }
                break;
            }
        }
    }
    grid->_originX = event->pos().x();
    grid->_originY = event->pos().y();

    changeGrid(_currentGridId);

    update();
}


bool GridWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        Grid* grid = _grids[_currentGridId];
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        float newX = mouseEvent->pos().x();
        float newY = mouseEvent->pos().y();
        QTransform translate(
            1, 0, 0, 
            0, 1, 0, 
            (newX - grid->_originX) / grid->_scale, (newY - grid->_originY) / grid->_scale, 1
        );
        grid->_originX = newX;
        grid->_originY = newY;
        grid->_transform = translate * grid->_transform;
        update();
    }

    return false;
}

void GridWidget::paint(int gridIndex, QPainter* qpainter, QPen* qpen) {
    Grid* grid = _grids[gridIndex];
    int gap = _imgWidth / 10;
    int margin = _imgWidth / 20;

    int numOfImages = grid->indices.size();
    int numOfColumns, numOfRows;
    if (numOfImages == 0) return;
    else if (numOfImages == 1) {
        numOfColumns = 1;
        numOfRows = 1;
    }
    else {
        numOfColumns = (int) sqrt(numOfImages) + 1;
        numOfRows = (numOfImages-1) / numOfColumns + 1;
    }
    grid->_ratio = numOfRows*1.0 / numOfColumns;

    qpainter->setWorldTransform(grid->_transform);
    float screenScaling = size().width() / (numOfColumns * (_imgWidth + gap));
    qpainter->scale(screenScaling, screenScaling);

    int cnt = 0;
    for (int index: grid->indices) {

        int row = cnt / numOfColumns;
        int col = cnt % numOfColumns;
        QImage* img = _mm->getImage(index);

        qpen->setWidth(96.0 / grid->_scale);
        qpainter->setPen(*qpen);
        if (gridIndex == _currentGridId) {
            qpainter->drawRect(
                margin + (gap + _imgWidth) * col, 
                margin + (gap + _imgHeight) * row, 
                _imgWidth, 
                _imgHeight
            );
        }
        qpainter->drawImage(
            margin + (gap + _imgWidth) * col, 
            margin + (gap + _imgHeight) * row, 
            (img == nullptr? emptyImage : *img)
        );
        cnt ++;
    }
}

void GridWidget::paintEvent(QPaintEvent* event) {
    int windowWidth = _parent->size().width();
    int windowHeight = _parent->size().height();
    resize(windowWidth, windowHeight);

    QPainter qpainter(this);
    QPen qpen;
    qpen.setColor(QColor(255, 0, 0, 48));

    for (int i=0; i<_gridCount; i++) {
        if (i == _currentGridId) continue;
        else paint(i, &qpainter, &qpen);
    }
    paint(_currentGridId, &qpainter, &qpen);
}