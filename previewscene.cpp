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

    AddAxesLines(imageGen);

    this->removeItem(&emItemGroup);
    this->addItem(&emItemGroup);
    invalidate(this->sceneRect(), QGraphicsScene::ItemLayer);
}

/** ****************************************************************************
 * @brief PreviewScene::AddAxesLines
 * @param imageGen
 */
void PreviewScene::AddAxesLines(ImageGen & imageGen) {
    this->removeItem(&yAxisItem);
    this->removeItem(&xAxisItem);
    // Axes are drawn only when interacting with an arrangement that's mirrored
    if (imageGen.act.IsActive() && imageGen.act.GetActiveGroup()) {
        EmArrangement* group = imageGen.act.GetActiveGroup();
        if (group->mirrorHor) {
            yAxisItem.setLine(QLineF(0, sceneRect().top(), 0, sceneRect().bottom()));
            this->addItem(&yAxisItem);
        }
        if (group->mirrorVert) {
            xAxisItem.setLine(QLineF(sceneRect().left(), 0, sceneRect().right(), 0));
            this->addItem(&xAxisItem);
        }
    }
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
    this->fitInView(imageGen.areaSim, Qt::KeepAspectRatio);
    // Background redraw will be triggered
}

/** ****************************************************************************
 * @brief PreviewView::BackgroundChanged
 */
void PreviewView::OnBackgroundChange(QImage & image, qreal imgPerSimUnit)
{
    scene()->invalidate(imageGen.areaSim, QGraphicsScene::BackgroundLayer);
    // resetCachedContent(); // Delete previously cached background to force redraw
    backgroundImage = &image;
    backgroundImgPerSimUnit = imgPerSimUnit;
    update(); // Redraws, but not immediately
}

/** ****************************************************************************
 * @brief PreviewView::drawBackground
 * @param painter
 * @param rect
 */
void PreviewView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // All painting is done in scene (simulation) coordinates!
    // rect is in scene coords
    // This function is called to draw partial backgrounds and complete backgrounds.
    // 'rect' indicates the background section to draw (in scene units)

    // Image coordinates map to the scene / simulation coordinates with
    // a factor of imgPerSimUnit. The image itself has dimensions that start at 0,0,
    // whilst the scene can start at any point.
    QRectF imgSourceRect((rect.topLeft() - sceneRect().topLeft()) * backgroundImgPerSimUnit,
                      rect.size() * backgroundImgPerSimUnit);
    painter->drawImage(rect, *backgroundImage, imgSourceRect);
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

/** ****************************************************************************
 * @brief PreviewScene::OnEmitterChange
 */
void PreviewScene::OnEmitterChange()
{
    AddEmitters(imageGen);
}

