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

/** ****************************************************************************
 * @brief PreviewScene::Cancel
 */
void PreviewScene::Cancel()
{
    // Restore the backup
    if (active) {
        if (grpActive) {
            *grpActive = grpBackup;
        }
        imageGen.setTargetImgPoints(backupImgPoints);
    }
    active = false;
}

/** ****************************************************************************
 * @brief PreviewScene::mousePressEvent
 * @param event
 */
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
    backupImgPoints = imageGen.targetImgPoints;
    imageGen.setTargetImgPoints(40000);
    grpBackup = *grpActive; // Save, so that it can be reverted
    pressPos = event->scenePos();
}

/** ****************************************************************************
 * @brief PreviewScene::mouseReleaseEvent
 * @param event
 */
void PreviewScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug("Scene release event (%7.2f, %7.2f)", event-> scenePos().x(), event->scenePos().y());
    imageGen.setTargetImgPoints(backupImgPoints);
    imageGen.GenerateImage(imageGen.image);
    this->invalidate(imageGen.simArea); // Forces redraw
    //this->views()[0]->resetCachedContent(); // Delete previously cached background to force redraw
    this->views()[0]->update();
    active = false;
}

void PreviewScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug("Scene move    event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    if (!active) {return;}
    // Determine how much the mouse has moved while clicked
    qreal moveDistY = event->scenePos().y() - pressPos.y();
    grpActive->arcRadius = std::max(0.0, grpBackup.arcRadius - moveDistY);
    AddEmitters(imageGen);
    imageGen.GenerateImage(imageGen.image);
    this->invalidate(imageGen.simArea); // Forces redraw
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
    pen.setWidthF(imageGen.s.emitterRadius * 0.3);
    QBrush brush(QColorConstants::White);

    for (QGraphicsItem * item : emItemGroup.childItems()) {
        emItemGroup.removeFromGroup(item);
        delete item;
    }

    // !@#$ upgrade to group together each arrangement

    // Note that the actual ellipse position is a combination of the QGraphicsItem
    // position and the QGraphicsEllipseItem rect position.
    // If the item pos is (0,0) and the rect() is centered on (10,20), then
    // the ellipse will be centered on (10,20).
    QRectF emRect(0., 0., emDia, emDia);
    emRect.moveCenter(QPointF(0,0)); // The rect() will always be centered around 0
    for (EmitterF e : emitters) {
        QGraphicsEllipseItem * item = new QGraphicsEllipseItem(emRect, &emItemGroup);
        item->setPos(e.loc);
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
    // All painting is done in scene (simulation) coordinates!
    // rect is in scene coords
    // This function is called to draw partial backgrounds and complete backgrounds.
    // 'rect' indicates the background section to draw (in scene units)

    // Image coordinates map to the scene / simulation coordinates with
    // a factor of imgPerSimUnit. The image itself has dimensions that start at 0,0,
    // whilst the scene can start at any point.
    QRectF imgSourceRect((rect.topLeft() - sceneRect().topLeft()) * imageGen.imgPerSimUnit,
                      rect.size() * imageGen.imgPerSimUnit);
    painter->drawImage(rect, imageGen.image, imgSourceRect);

    painter->restore();
}

/** ****************************************************************************
 * @brief PreviewScene::ListAllItems
 */
void PreviewScene::ListAllItems() {
    QStringList strList;
    strList.append(QString::asprintf("%d items in scene. Locs = ", items().size()));
    for (auto item : items()) {
        QPointF scenePos = item->mapToScene(item->pos());
        strList.append(QString::asprintf("(%.2f, %.2f), ", scenePos.x(), scenePos.y()));
    }
    qDebug() << strList.join("");
}

