
#include "preview.h"
#include "imagegen.h"
#include "interact.h"

Preview::Preview(QWidget *parent) : QWidget(parent)
{


}

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
}


