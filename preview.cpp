
#include "preview.h"
#include "imagegen.h"

Preview::Preview(QWidget *parent) : QWidget(parent)
{


}

void Preview::paintEvent(QPaintEvent *event)
{
    (void) event;
    imageGen.drawPreview(this);
}


