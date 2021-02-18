#include "previewscene.h"
#include "imagegen.h"
#include "mainwindow.h"
#include "interact.h"
#include <QDebug>
#include <QResizeEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGradient>
#include <QGuiApplication>
#include <QScreen>

/** ****************************************************************************
 * @brief PreviewScene::PreviewScene
 * @param parent
 */
PreviewScene::PreviewScene(QObject *parent, MainWindow &mainWindowIn) :
    QGraphicsScene(parent),
    mainWindow(mainWindowIn)
{

}

/** ****************************************************************************
 * @brief PreviewScene::mousePressEvent
 * @param event
 */
void PreviewScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    mainWindow.interact.mousePressEvent(event, this);}

void PreviewScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    mainWindow.interact.mouseReleaseEvent(event, this);}

void PreviewScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    mainWindow.interact.mouseMoveEvent(event, this);}

/** ****************************************************************************
 * @brief PreviewScene::EmitterArngmtToList
 */
void PreviewScene::EmitterArngmtToList(ImageGen & imgGen) {
    // Get the vector of emitters
    QVector<EmitterF> emitters;
    if (imgGen.GetEmitterList(emitters)) {
        return;
    }

    const double emDia = imgGen.s.emitterRadius * 2.0; // Simulation/scene coordinates
    QPen pen(QColorConstants::Black);
    pen.setWidthF(imgGen.s.emitterRadius * 0.3);
    QBrush brush(QColorConstants::White);

    // Delete every existing graphics item from the group
    for (QGraphicsItem * item : emItemGroup.childItems()) {
        emItemGroup.removeFromGroup(item);
        delete item;
    }

    if (!imgGen.EmittersHidden()) {
        // For each emitter, create a graphics item and add to the group
        // !@# upgrade to group together each arrangement
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
    }


    // Re-add the group to the scene
    this->removeItem(&emItemGroup);
    this->addItem(&emItemGroup);

    AddAxesLines();

    invalidate(this->sceneRect(), QGraphicsScene::ItemLayer);
    // The scene and background are automatically redrawn
}

/** ****************************************************************************
 * @brief PreviewScene::AddAxesLines
 * @param imageGen
 */
void PreviewScene::AddAxesLines() {
    Interact & act = mainWindow.interact;
    this->removeItem(&yAxisItem);
    this->removeItem(&xAxisItem);
    // Axes are drawn only when interacting with an arrangement that's mirrored
    if (act.IsActive() && act.GetActiveArrangement()) {
        EmArrangement* group = act.GetActiveArrangement();
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

    if (imageGen.s.view.aspectRatio < 1.) {
        // Portrait orientation. Width determined from height
        widthFromHeight = true;
        this->setMinimumHeight(300);
        this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    }
    else {
        // Landscape orientation. Height determined from width
        widthFromHeight = false;
        this->setMinimumWidth(300);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    }

    this->setContentsMargins(0,0,0,0);
}

/** ****************************************************************************
 * @brief PreviewView::resizeEvent
 * @param event
 */
void PreviewView::resizeEvent(QResizeEvent *event) {
    // Enforce the aspect ratio
    // 2020-12-29 This causes jerky behaviour as it triggers this resizeEvent again. Does not work well.
    //qreal newWidth = std::min((qreal)event->size().height() * imageGen.aspectRatio(), (qreal)event->size().width());
    //qDebug("resizeEvent. size=(%dx%d)px. newWidth=%.2f. viewWidth=%d", event->size().width(), event->size().height(), (qreal)event->size().height() * imageGen.s.view.aspectRatio, this->width());
    // this->resize(newWidth, newWidth / imageGen.aspectRatio());

    // Ensure that the viewable section of the screen stays the same
    bool expanded;
    if (widthFromHeight) {
        qreal newWidth = (qreal)event->size().height() * imageGen.s.view.aspectRatio;
        // Limit width to 60% of the screen width
        newWidth = std::min(newWidth, 0.60 * (qreal)QGuiApplication::primaryScreen()->geometry().width());
        expanded = newWidth > this->minimumWidth();
        this->setMinimumWidth(newWidth);
    }
    else { // Height from width
        qreal newHeight = (qreal)event->size().width() / imageGen.s.view.aspectRatio;
        // Limit height to 85% of the screen height
        newHeight = std::min(newHeight, 0.85 * (qreal)QGuiApplication::primaryScreen()->geometry().height());
        expanded = newHeight > this->minimumHeight();
        this->setMinimumHeight(newHeight);
//                qDebug("resizeEvent. size=(%dx%d)px. newHeight=%.2f. height=%d. Expanded=%d",
//                       event->size().width(), event->size().height(), newHeight, this->height(), expanded);
    }

    if (expanded) {
        this->fitInView(imageGen.areaSim, Qt::KeepAspectRatioByExpanding);
    }else {
        this->fitInView(imageGen.areaSim, Qt::KeepAspectRatio);
    }

    // Set the target image points to the new size
    imageGen.setTargetImgPoints(this->size().width() * this->size().height(), imageGen.genPreview);

    QGraphicsView::resizeEvent(event);
    // Background redraw will be triggered
}

/** ****************************************************************************
 * @brief PreviewView::OnPatternImageChange
 */
void PreviewView::OnPatternImageChange(QImage & image, qreal imgPerSimUnit, QColor backgroundClr)
{
    scene()->invalidate(imageGen.areaSim, QGraphicsScene::BackgroundLayer);
    // resetCachedContent(); // Delete previously cached background to force redraw
    patternImage = &image;
    patternImgPerSimUnit = imgPerSimUnit;
    backgroundColour = backgroundClr;
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

    painter->fillRect(sceneRect(), backgroundColour);
    if (0) { // Draw only the requested portion of the background
        // This is somewhat unreliable (Dean R 2021-02-17)
        QRectF imgSourceRect((rect.topLeft() - sceneRect().topLeft()) * patternImgPerSimUnit,
                             rect.size() * patternImgPerSimUnit);
        painter->drawImage(rect, *patternImage, imgSourceRect);
        //        qDebug("drawBackground. rect=(%.2fx%.2f). @(%.2f, %.2f). imgSrcRect=(%.2fx%.2f). viewSz=(%dx%d)",
        //               rect.width(), rect.height(), rect.x(), rect.y(), imgSourceRect.width(), imgSourceRect.height(),
        //               this->width(), this->height());
    }else { // Redraw the entire background (seems more resiliant with various types of resizing)
        painter->drawImage(sceneRect(), *patternImage, patternImage->rect());
//                qDebug("drawBackground. rect=(%.2fx%.2f). @(%.2f, %.2f). viewSz=(%dx%d)",
//                       rect.width(), rect.height(), rect.x(), rect.y(),
//                       this->width(), this->height());
    }
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
 * @brief PreviewScene::OnEmitterArngmtChange
 */
void PreviewScene::OnEmitterArngmtChange()
{
    EmitterArngmtToList(imageGen);
}

/** ****************************************************************************
 * @brief PreviewScene::OverlayTextSlot displays text on top of the scene
 * @param text
 */
void PreviewScene::OverlayTextSlot(QString text)
{
    if (text.isEmpty()) {
        textOverlay.setVisible(false);
        return;
    }
    textOverlay.setPlainText(text);
    textOverlay.setVisible(true);
    textOverlay.setTextWidth(sceneRect().width() * 0.9);
    textOverlay.setPos(sceneRect().left() + sceneRect().width() * 0.05, -10);
    QFont font = textOverlay.font();
    font.setPointSize(7);
    textOverlay.setFont(font);
    textOverlay.setDefaultTextColor(Qt::gray);
    this->addItem(&textOverlay);
 }

