#include "previewscene.h"
#include "imagegen.h"
#include <QDebug>
#include <QResizeEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGradient>

PreviewScene::PreviewScene(QObject *parent) :
    QGraphicsScene(parent)
{

}

void PreviewScene::Cancel()
{
    // Restore the backup
    if (active) {
        if (grpActive) {
            *grpActive = grpBackup;
        }
    }
    active = false;
}


void PreviewScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug("Scene press   event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    Cancel();
    grpActive = imageGen.GetActiveArrangement();
    if (!grpActive) {
        // No active groups
        return;
    }
    active = true;
    grpBackup = *grpActive; // Save, so that it can be reverted
    pressPos = event->scenePos();
}

void PreviewScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug("Scene release event (%7.2f, %7.2f)", event-> scenePos().x(), event->scenePos().y());
    imageGen.GenerateImage(imageGen.image);
    this->invalidate(imageGen.simArea); // Forces redraw
    this->views()[0]->resetCachedContent(); // Delete previously cached background to force redraw
    this->views()[0]->update();
    active = false;
}

void PreviewScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug("Scene move    event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    if (!active) {return;}
    // Determine how much the mouse has moved while clicked
    qreal moveDistY = pressPos.y() - event->scenePos().y();
    grpActive->arcRadius = std::max(0.0, grpBackup.arcRadius + moveDistY);
    AddEmitters(imageGen);
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

    const double emDia = imageGen.s.emitterRadius * 2.0; // Simulation/scene coordinates
    QPen pen(QColorConstants::Black);
    pen.setWidthF(0.5);
    QBrush brush(QColorConstants::White);

    for (QGraphicsItem * item : emItemGroup.childItems()) {
        emItemGroup.removeFromGroup(item);
        delete item;
    }

    // !@#$ upgrade to group together each arrangement
    QRectF emRect(0., 0., emDia, emDia);
    for (EmitterF e : emitters) {
        emRect.moveCenter(e.loc);
        QGraphicsEllipseItem * item = new QGraphicsEllipseItem(emRect, &emItemGroup);
        item->setPen(pen);
        item->setBrush(brush);
    }

    this->removeItem(&emItemGroup);
    this->addItem(&emItemGroup);
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

