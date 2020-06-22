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

PreviewView::PreviewView(QWidget *parent) : QGraphicsView(parent)
{
}

void PreviewView::resizeEvent(QResizeEvent *event)
{
    event->accept();
    qreal newWidth = std::min((qreal)event->size().height() * imageGen.aspectRatio(), (qreal)event->size().width());
    QWidget::resize(newWidth, newWidth / imageGen.aspectRatio());
}
