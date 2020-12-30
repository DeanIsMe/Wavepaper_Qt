#ifndef PREVIEWSCENE_H
#define PREVIEWSCENE_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include "imagegen.h"

/** ****************************************************************************
 * @brief The PreviewScene class is the canvas upon which the preview is painted
 * The class holds GraphicsItems for emitters and such, and also handles
 * interaction.
 */
class PreviewScene : public QGraphicsScene
{
    Q_OBJECT
private:
    QGraphicsLineItem yAxisItem;
    QGraphicsLineItem xAxisItem;
    void Cancel();
public:
    explicit PreviewScene(QObject *parent = nullptr);
    QGraphicsItemGroup emItemGroup;
    void ListAllItems();

public slots:
    void OnEmitterArngmtChange();

    // QGraphicsScene interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        imageGen.act.mousePressEvent(event, this);}
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        imageGen.act.mouseReleaseEvent(event, this);}
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override {
        imageGen.act.mouseMoveEvent(event, this);}

    // Custom
public:
    void EmitterArngmtToList(ImageGen &imageGen);
    void AddAxesLines(ImageGen &imageGen);

};

/** ****************************************************************************
 * @brief The PreviewView class
 */
class PreviewView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PreviewView(QWidget *parent = nullptr);
    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void OnPatternImageChange(QImage &image, qreal imgPerSimUnit, QColor backgroundClr);

    // QGraphicsView interface
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);

protected:
    bool widthFromHeight; // True if the width is determined by the height. False for the opposite.
    QImage * patternImage = nullptr;
    qreal patternImgPerSimUnit; // Saved for the pattern image
    QColor backgroundColour = Qt::black;
};

#endif // PREVIEWSCENE_H
