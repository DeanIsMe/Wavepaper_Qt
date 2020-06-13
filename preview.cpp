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
    QRgb * pixData = new QRgb[this->width()*this->height()];
    QImage img((uchar*)pixData, this->width(), this->height(), QImage::Format_ARGB32); // QRgb is ARGB32 (8 bits per channel)

    // Fill in the image data
    if (0) {
        // Test gradient
        for (int y = 0; y < img.height(); y++) {
            int hOff = y * img.width();
            for (int x = 0; x < img.width(); x++) {
                pixData[hOff + x] = qRgb(0, x * 255 / img.width(), y * 255 / img.height());
            }
        }
    }
    else {
        // Image Gen
        ImageGen imgGen;
        imgGen.draw(img.width(), img.height(), pixData);
    }

    QPainter painter(this);
    painter.drawImage(0, 0, img);
    delete pixData;
}


