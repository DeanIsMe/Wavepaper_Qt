#include <QImage>
#include <QPainter>
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

    RectToQString(this->rect());
    qDebug() << "Preview window is " << RectToQString(this->rect());
    qDebug() << "   View window is " << RectToQString(viewWindow);

    QPainter painter(this);
    painter.setWindow(viewWindow);

    QImage img((uchar*)pixArr.getDataPtr(), pixArr.width, pixArr.height, QImage::Format_ARGB32); // QRgb is ARGB32 (8 bits per channel)

    painter.drawImage(viewWindow.x(), viewWindow.y(), img);
}


