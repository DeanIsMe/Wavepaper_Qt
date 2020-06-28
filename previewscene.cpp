#include "previewscene.h"
#include "imagegen.h"
#include <QDebug>
#include <QResizeEvent>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGradient>

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
 * @brief PreviewScene::AddEmitters
 */
void PreviewScene::AddEmitters(ImageGen & imageGen) {
    // Get the vector of emitters
    QVector<EmitterF> emitters;
    if (imageGen.GetEmitterList(emitters)) {
        return;
    }

    QList<QGraphicsItem *> emItemList;

    const double emDia = imageGen.s.emitterRadius * 2.0; // Simulation/scene coordinates
    QPen pen(QColorConstants::Black);
    pen.setWidthF(0.5);
    QBrush brush(QColorConstants::White);

    if (emItemGroup) {
        this->destroyItemGroup(emItemGroup);
    }

    QRectF emRect(0., 0., emDia, emDia);
    for (EmitterF e : emitters) {
        emRect.moveCenter(e.loc);
        emItemList.append(this->addEllipse(emRect, pen, brush));
    }
    emItemGroup = this->createItemGroup(emItemList);
}

/** ****************************************************************************
 * @brief PreviewView::PreviewView constructor
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
void PreviewView::resizeEvent(QResizeEvent *event) {
    event->accept();
    // Enforce the aspect ratio
    qreal newWidth = std::min((qreal)event->size().height() * imageGen.aspectRatio(), (qreal)event->size().width());
    this->resize(newWidth, newWidth / imageGen.aspectRatio());
    // Ensure that the viewable section of the screen stays the same
    this->fitInView(imageGen.simArea, Qt::KeepAspectRatio);
    qDebug("resizeEvent DONE");
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

    // All painting is done in scene (simulation) coordinates!
    // rect is in scene coords

    painter->setViewport(this->rect());
    painter->setWindow(painter->viewport());
    painter->drawImage(rect, image);

    // The above is the code that works. I don't understand exactly why...
    // It doesn't seem right to me.
    // The painter paints on scene coordinates.
    // Anyway, I've wasted hours on this which is more than enough.

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

/** ****************************************************************************
 * @brief PreviewScene::ListAllItems
 */
void PreviewScene::ListAllItems() {
    QStringList strList;
    strList.append(QString::asprintf("%d items in scene. Locs = ", items().size()));
    for (auto item : items()) {
        strList.append(QString::asprintf("(%.2f, %.2f), ", item->x(), item->y()));
    }
    qDebug() << strList.join("");
}
