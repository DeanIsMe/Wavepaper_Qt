#include <QImage>
#include <QPainter>
#include <QDebug>
#include "preview.h"
#include "imagegen.h"

Preview::Preview(QWidget *parent) : QWidget(parent)
{



}

void Preview::paintEvent(QPaintEvent *event)
{
    qDebug("Paint preview");
    (void) event;

    QRect viewWindow(0, 0, this->width(), this->height());
    //QRect viewWindow(0, 0, 100, 200);
    viewWindow.moveCenter(QPoint(0,0));

    Rgb2D_C pixArr(viewWindow);

    // Fill in the image data
    if (0) {
        // Test gradient
        for (int y = pixArr.yTop; y < pixArr.yTop + pixArr.height; y++) {
            for (int x = pixArr.xLeft; x < pixArr.xLeft + pixArr.width; x++) {
                pixArr.setPoint(x, y, qRgb(0, x * 255 / pixArr.width, y * 255 / pixArr.height));
            }
        }
    }
    else {
        // Image Gen
        ImageGen imgGen;
        imgGen.generateImage(pixArr);
    }


    QPainter painter(this);
    painter.setPen(QColor(255,0,0));
    painter.drawRect(QRect(0,0,10,10));

    painter.setWindow(viewWindow);

    QImage img((uchar*)pixArr.getDataPtr(), pixArr.width, pixArr.height, QImage::Format_ARGB32); // QRgb is ARGB32 (8 bits per channel)

    painter.setPen(QColorConstants::Blue);
    painter.drawRect(QRect(0,0,10,10));

    painter.drawImage(viewWindow.x(), viewWindow.y(), img);
}


