#ifndef PREVIEWSCENE_H
#define PREVIEWSCENE_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include "imagegen.h"
#include "interact.h"

/** ****************************************************************************
 * @brief The PreviewScene class is the canvas upon which the preview is painted
 * The class holds GraphicsItems for emitters and such, and also handles
 * interaction.
 */
class PreviewScene : public QGraphicsScene
{
    Q_OBJECT
private:
    MainWindow& mainWindow;
    QGraphicsLineItem yAxisItem;
    QGraphicsLineItem xAxisItem;
    QGraphicsTextItem textOverlay;
    void Cancel();
public:
    explicit PreviewScene(QObject *parent, MainWindow& mainWindowIn);
    QGraphicsItemGroup emItemGroup;
    void ListAllItems();

public slots:
    void OnEmitterArngmtChange();
    void OverlayTextSlot(QString text);

    // QGraphicsScene interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    // Custom
public:
    void EmittersToGraphItems(ImageGen &imgGen);
    void AddAxesLines();
};

/** ****************************************************************************
 * @brief The PreviewView class
 */
class PreviewView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PreviewView(QWidget *parent, MainWindow& mainWindowIn);
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void OnPatternImageChange(QImage &image, qreal imgPerSimUnit, QColor backgroundClr);

    // QGraphicsView interface
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void OnAspectRatioChange();

protected:
    MainWindow& mainWindow;
    bool widthFromHeight; // True if the width is determined by the height. False for the opposite.
    QImage * patternImage = nullptr;
    qreal patternImgPerSimUnit; // Saved for the pattern image
    QColor backgroundColour = Qt::black;
};

#endif // PREVIEWSCENE_H
