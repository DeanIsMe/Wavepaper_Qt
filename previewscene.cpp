#include "previewscene.h"
#include "imagegen.h"
#include <QDebug>
#include <QResizeEvent>

PreviewScene::PreviewScene(QObject *parent) : QGraphicsScene(parent)
{

}

void PreviewScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug("Scene Drag enter");
}

void PreviewScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug("Scene Drag move");
}

void PreviewScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    qDebug("Scene Drag leave");
}

/** ****************************************************************************
 * @brief PreviewView::PreviewView
 * @param parent
 */
PreviewView::PreviewView(QWidget *parent) : QGraphicsView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/** ****************************************************************************
 * @brief PreviewView::resizeEvent
 * @param event
 */
void PreviewView::resizeEvent(QResizeEvent *event)
{
    event->accept();
    qreal newWidth = std::min((qreal)event->size().height() * imageGen.aspectRatio(), (qreal)event->size().width());
    this->resize(newWidth, newWidth / imageGen.aspectRatio());
}

/** ****************************************************************************
 * @brief PreviewView::drawBackground
 * @param painter
 * @param rect
 */
void PreviewView::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->save();
    QImage & image = imageGen.image;

    painter->setViewport(this->rect());
    painter->setWindow(painter->viewport());
    painter->drawImage(rect.toRect(), image);

    // The above is the code that works. I don't understand exactly why...
    // It doesn't seem right to me. I think that drawImage uses PaintDevice coordinates
    // instead of logical coordinates for it's x,y.
    // Anyway, I've wasted hours on this which is more than enough.

    // Alternative:
    // QRect imgArea = image.rect();
    // painter->setWindow(imgArea);
    // // painter->setViewport(0, 0, FP_TO_INT(rect.width()), FP_TO_INT(rect.height())); // Alternative
    // painter->setViewport(this->rect());
    // painter->drawImage(FP_TO_INT(rect.x()), FP_TO_INT(rect.y()), image);

    // Example values (very wide window)
    // Exposed rect (input)=  "166.0 x 297.0 @(-157.0, -280.0)"
    // PreviewView rect    =  "167 x 298 @(0, 0)"
    // Image rect          =  "237 x 422 @(0, 0)"
    // Before window & viewport transform:
    // Paint Window        =  "1109 x 298 @(0, 0)"
    // Paint Viewport      =  "1109 x 298 @(0, 0)"
    // After transform:
    // Paint Window        =  "237 x 422 @(0, 0)"
    // Paint Viewport      =  "167 x 298 @(0, 0)"

    painter->restore();
}
