#ifndef PREVIEWSCENE_H
#define PREVIEWSCENE_H

#include <QGraphicsScene>
#include <QGraphicsView>

/*
 *
void Preview::paintEvent(QPaintEvent *event)
{
    (void) event;
    imageGen.DrawPreview(this);
}

void Preview::mousePressEvent(QMouseEvent *event)
{
    interact.mousePressEvent(event);
}

void Preview::mouseReleaseEvent(QMouseEvent *event)
{
    interact.mouseReleaseEvent(event);
}

void Preview::mouseMoveEvent(QMouseEvent *event)
{
    interact.mouseMoveEvent(event);
    imageGen.DrawEmitters(this);
}

 *
 */

class PreviewScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit PreviewScene(QObject *parent = nullptr);

signals:

    // QGraphicsScene interface
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;

//    void mousePressEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
};


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
};

#endif // PREVIEWSCENE_H
