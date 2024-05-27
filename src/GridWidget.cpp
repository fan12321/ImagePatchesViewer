#include "GridWidget.h"
#include <QImage>
#include <cmath>

GridWidget::GridWidget(QWidget* parent, QString imageDir, MemoryManager* mm) : 
    QWidget(parent),
    _parent(parent),
    _imageDir(imageDir),
    _mm(mm)
{
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

    // _mm->setMaxImagesInCache(3);
    _mm->setMaxImagesInCache(_maxImagesInCache);

    emptyImage.fill(QColor(0, 0, 0, 0));

    _grids.push_back(new Grid(parent->size().width()));
    _grids.push_back(new Grid(parent->size().width()));
    _focusGrid = 0;

    show();
};


void GridWidget::wheelEvent(QWheelEvent* event)
{
    auto scroll = event->angleDelta().y();
    if (scroll != 0) {
        float scale = (event->angleDelta().y() > 0? 1.1: 0.91);
        Grid* grid = _grids[_focusGrid];

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
    Grid* grid;
    for (int i=0; i<2; i++) {
        grid = _grids[i];
        if (grid->inside(event->pos())) {
            _focusGrid = i;
            break;
        }
    }
    grid = _grids[_focusGrid];
    grid->_originX = event->pos().x();
    grid->_originY = event->pos().y();

    update();
}


bool GridWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        Grid* grid = _grids[_focusGrid];
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

void GridWidget::paintEvent(QPaintEvent* event) {
    int windowWidth = _parent->size().width();
    int windowHeight = _parent->size().height();
    resize(windowWidth, windowHeight);
    int gap = _imgWidth / 10;
    int margin = _imgWidth / 20;

    QPainter qpainter(this);
    QPen qpen;
    qpen.setColor(QColor(255, 0, 0, 64));
    qpen.setWidth(32);
    qpainter.setPen(qpen);

    for (int i=0; i<2; i++) {
        Grid* grid = _grids[i];

        int numOfImages = _mm->indices.size();
        int numOfColumns, numOfRows;
        if (numOfImages == 0) break;
        else if (numOfImages == 1) {
            numOfColumns = 1;
            numOfRows = 1;
        }
        else {
            numOfColumns = (int) sqrt(numOfImages) + 1;
            numOfRows = (numOfImages-1) / numOfColumns + 1;
        }
        grid->_ratio = numOfRows*1.0 / numOfColumns;

        qpainter.setWorldTransform(grid->_transform);
        float screenScaling = size().width() / (numOfColumns * (_imgWidth + gap));
        qpainter.scale(screenScaling, screenScaling);

        int cnt = 0;
        for (int index: _mm->indices) {

            int row = cnt / numOfColumns;
            int col = cnt % numOfColumns;
            QImage* img = _mm->getImage(index);

            if (i == _focusGrid) {
                qpainter.drawRect(
                    margin/2 + (gap + _imgWidth) * col, 
                    margin/2 + (gap + _imgHeight) * row, 
                    gap/2 + _imgWidth, 
                    gap/2 + _imgHeight
                );
            }
            qpainter.drawImage(
                margin + (gap + _imgWidth) * col, 
                margin + (gap + _imgHeight) * row, 
                (img == nullptr? emptyImage : *img)
            );
            cnt ++;
        }
    }
}