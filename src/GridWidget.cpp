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
    // 5GB of images in cache, wierd calculation order to prevent overflow
    _maxImagesInCache = (int) ceil(5.0 * 1024.0 * (1024.0 / _imgWidth) * (1024.0 / _imgHeight) * (1.0 / sizeof(*img)));
    delete img;
    _mm->setMaxImagesInCache(_maxImagesInCache);

    // qDebug() << _maxImagesInCache;

    show();
};

void GridWidget::unloadImages(std::set<int>& s) {
    for (int index: s) {
        _mm->unLoadImage(index);
    }
}

void GridWidget::loadImages(std::set<int>& s) {
    for (int index: s) {
        _mm->loadImage(index);
    }
}

void GridWidget::wheelEvent(QWheelEvent* event)
{
    auto scroll = event->angleDelta().y();
    if (scroll != 0) {
        float scale = (event->angleDelta().y() > 0? 1.1: 0.91);

        _scale *= scale;

        QPoint transformedMousePosition = _transform.inverted().map(mapFromGlobal(QCursor::pos()));
        _x = transformedMousePosition.x();
        _y = transformedMousePosition.y();

        // column major
        QTransform t1(
            1, 0, 0,
            0, 1, 0, 
            _x, _y, 1    
        );
        QTransform s(
            scale, 0, 0, 
            0, scale, 0, 
            0, 0, 1
        );
        QTransform t2(
            1, 0, 0,
            0, 1, 0, 
            -_x, -_y, 1    
        );
        _transform = t2 * s * t1 * _transform;
        update();
    }
}

void GridWidget::mousePressEvent(QMouseEvent* event) {
    _originX = event->pos().x();
    _originY = event->pos().y();
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
            (newX - _originX) / _scale, (newY - _originY) / _scale, 1
        );
        _originX = newX;
        _originY = newY;
        _transform = translate * _transform;
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

    int numOfImages = _mm->indices.size();
    if (numOfImages == 0) return;

    int numOfColumns = numOfImages > 5? 5: numOfImages;
    int cnt = 0;

    QPainter qpainter(this);

    qpainter.setWorldTransform(_transform);

    float screenScaling = size().width() / (numOfColumns * (_imgWidth + gap));
    qpainter.scale(screenScaling, screenScaling);

    for (int index: _mm->indices) {

        int row = cnt / numOfColumns;
        int col = cnt % numOfColumns;
        QImage* img = _mm->getImage(index);

        qpainter.drawImage(
            margin + (gap + _imgWidth) * col, 
            margin + (gap + _imgHeight) * row, 
            *img
        );
        cnt ++;
    }
}