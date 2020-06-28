#ifndef PREVIEWSCENE_H
#define PREVIEWSCENE_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
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
    // Interaction:
    bool active = false;
    EmArrangement grpBackup;
    EmArrangement * grpActive;
    QPointF pressPos;
    void Cancel();

public:
    explicit PreviewScene(QObject *parent = nullptr);
    QGraphicsItemGroup emItemGroup;
    void ListAllItems();

signals:

    // QGraphicsScene interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    // Custom
public:
    void AddEmitters(ImageGen &imageGen);

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


    // QGraphicsView interface
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);

    // QWidget interface
protected:
};

#endif // PREVIEWSCENE_H
