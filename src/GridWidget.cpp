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

    int imageBytes = img->sizeInBytes();
    delete img;

    // 5GB cache, 30GB max of total images loaded
    int cacheSize = floor(5.0 * 1024.0 * 1024.0 * 1024.0 * (1.0 / imageBytes));
    int maxImagesLoaded = floor(30.0 * 1024.0 * 1024.0 * 1024.0 * (1.0 / imageBytes));
    _mm->setMaxCacheSize(cacheSize);
    _mm->setMaxImagesLoaded(maxImagesLoaded);

    emptyImage.fill(QColor(0, 0, 0, 0));

    _currentGrid = new Grid(parent->size().width());
    _gridCount = 1;


    action1 = new QAction("New selection", this);
    connect(action1, SIGNAL(triggered()), this, SLOT(newSelection()));
    action2 = new QAction("Delete selection", this);
    connect(action2, SIGNAL(triggered()), this, SLOT(deleteSelection()));
    action3 = new QAction("Relative position", this);
    action3->setCheckable(true);
    connect(action3, SIGNAL(triggered()), this, SLOT(toggleLayout()));

    show();
};

GridWidget::~GridWidget() {
    delete _mm;
    for (int i=0; i<_gridCount; i++) {
        _currentGrid = _currentGrid->_next;
        delete _currentGrid->_previous;
    }
}

void GridWidget::setIndices(std::vector<unsigned int> indices) {
    _currentGrid->indices = indices;
}

std::vector<unsigned int> GridWidget::getIndices() {
    return _currentGrid->indices;
}

void GridWidget::ShowContextMenu(const QPoint& pos) {
    QMenu contextMenu(tr("Context menu"), this);

    action1->setEnabled(_gridCount < MAX_SELECTION);
    action2->setEnabled(_gridCount != 1);

    contextMenu.addAction(action1);
    contextMenu.addAction(action2);
    contextMenu.addAction(action3);


    contextMenu.exec(mapToGlobal(pos));
}

void GridWidget::changeGrid(Grid* grid) {
    if (_currentGrid->indices.size() == 0) {
        _currentGrid->removeFromLinkedList();
        delete _currentGrid;
        _gridCount -= 1;
    }
    _currentGrid = grid;
    _points->setSelectionIndices(grid->indices);
    action3->setChecked(_currentGrid->_originPosition);
    mv::events().notifyDatasetDataSelectionChanged(_points);
}

void GridWidget::newSelection() {
    rearrange();
    Grid* newGrid = new Grid(_parent->size().width());
    newGrid->insertAfter(_currentGrid);

    _gridCount += 1;
    changeGrid(newGrid);
}

void GridWidget::rearrange() {
    Grid* it = _currentGrid;
    float newScale = MIN(0.8 / _gridCount, 0.5);
    for (int i=0; i<_gridCount; i++) {
        float newX = i * (1.0 / _gridCount) * _parent->size().width();
        float newY = 0.55 * _parent->size().height();

        it->_scale = newScale;
        it->_transform = QTransform(
            newScale, 0, 0, 
            0, newScale, 0, 
            newX, newY, 1
        );
        it = it->_next;
    }
    update();
}

void GridWidget::deleteSelection() {
    _mm->unloadImages(_currentGrid->indices);
    Grid* previousGrid = _currentGrid->_previous;
    _currentGrid->removeFromLinkedList();
    delete _currentGrid;
    _gridCount -= 1;
    _currentGrid = previousGrid;
    changeGrid(_currentGrid);
    update();
}

void GridWidget::toggleLayout() {
    int h = 221;
    int w = 186;
    if (_currentGrid->_originPosition) {
        _currentGrid->_originPosition = false;
        _currentGrid->_leftOffset = w;
        _currentGrid->_topOffset = h;
        update();
    }
    else {
        _currentGrid->_leftOffset = w;
        _currentGrid->_topOffset = h;
        for (int idx: _currentGrid->indices) {
            if (idx / w < _currentGrid->_topOffset) _currentGrid->_topOffset = idx / w;
            if (idx % w < _currentGrid->_leftOffset) _currentGrid->_leftOffset = idx % w;
        }
        _currentGrid->_originPosition = true;
        update();
    }
}


void GridWidget::wheelEvent(QWheelEvent* event)
{
    auto scroll = event->angleDelta().y();
    if (scroll != 0) {
        float scale = (event->angleDelta().y() > 0? 1.1: 0.91);

        _currentGrid->_scale *= scale;

        QPoint transformedMousePosition = _currentGrid->_transform.inverted().map(mapFromGlobal(QCursor::pos()));
        _currentGrid->_x = transformedMousePosition.x();
        _currentGrid->_y = transformedMousePosition.y();

        // column major
        QTransform t1(
            1, 0, 0,
            0, 1, 0, 
            _currentGrid->_x, _currentGrid->_y, 1    
        );
        QTransform s(
            scale, 0, 0, 
            0, scale, 0, 
            0, 0, 1
        );
        QTransform t2(
            1, 0, 0,
            0, 1, 0, 
            -_currentGrid->_x, -_currentGrid->_y, 1    
        );
        _currentGrid->_transform = t2 * s * t1 * _currentGrid->_transform;
        update();
    }
}

void GridWidget::mousePressEvent(QMouseEvent* event) {
    // disable mouse click at the beginning
    if (_currentGrid->indices.size() == 0 && _gridCount == 1) return;

    Grid* grid = _currentGrid;
    if (_currentGrid->indices.size() == 0) grid = grid->_next; 
    for (int i=0; i<_gridCount; i++) {
        if (grid->inside(event->pos())) {
            break;
        }
        grid = grid->_next;
    }
    grid->_originX = event->pos().x();
    grid->_originY = event->pos().y();

    changeGrid(grid);
    update();
}


bool GridWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        float newX = mouseEvent->pos().x();
        float newY = mouseEvent->pos().y();
        QTransform translate(
            1, 0, 0, 
            0, 1, 0, 
            (newX - _currentGrid->_originX) / _currentGrid->_scale, (newY - _currentGrid->_originY) / _currentGrid->_scale, 1
        );
        _currentGrid->_originX = newX;
        _currentGrid->_originY = newY;
        _currentGrid->_transform = translate * _currentGrid->_transform;
        update();
    }

    return false;
}

void GridWidget::paint(Grid* grid, QPainter* qpainter, QPen* qpen) {
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
    int w = 186;
    for (int index: grid->indices) {

        int row, col;
        if (grid->_originPosition) {
            row = index / w - grid->_topOffset;
            col = index % w - grid->_leftOffset;
        }
        else {
            row = cnt / numOfColumns;
            col = cnt % numOfColumns;
        }
        QImage* img = _mm->getImage(index);

        qpen->setWidth(gap);
        qpainter->setPen(*qpen);
        if (grid == _currentGrid) {
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
    QPen qpen, qpen_background;
    qpen.setColor(QColor(255, 180, 180, 230));
    qpen.setJoinStyle(Qt::MiterJoin);

    Grid* grid = _currentGrid;
    for (int i=0; i<_gridCount; i++) {
        grid = grid->_next;
        paint(grid, &qpainter, &qpen);
    }
}