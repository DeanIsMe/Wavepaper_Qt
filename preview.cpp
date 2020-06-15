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

    QRectF simWindow(0, 0, 100, 100. * 1920./1080.);
    simWindow.moveCenter(QPoint(0,0));

    // Determine the viewing window in image coordinates
    double targetImgPoints = 100000; // Total number of points in the preview


    double imgPerSimUnit = sqrt(targetImgPoints / simWindow.width() / simWindow.height());
    QRect imgRect(FP_TO_INT(simWindow.x() * imgPerSimUnit),
                  FP_TO_INT(simWindow.y() * imgPerSimUnit),
                  FP_TO_INT(simWindow.width() * imgPerSimUnit),
                  FP_TO_INT(simWindow.height()) * imgPerSimUnit);

    if (abs(simWindow.width() / simWindow.height() / (qreal)imgRect.width() * (qreal)imgRect.height() - 1) > 0.02) {
        qFatal("generateImage: viewWindow and simWindow are different ratios!");
        return;
    }
    state.imgPerSimUnit = imgPerSimUnit;


    // Emitter locations
    QList<QPointF> emLocs;
    emLocs.append(QPointF(-30, 0));
    emLocs.append(QPointF(0, 0));
    emLocs.append(QPointF(30, 0));

    if (emLocs.size() == 0) {
        qWarning("No emitters! Abort drawing");
        return;
    }

    // Convert emLocs to image coordinates
    QList<QPoint> emLocsImg;
    for (QPointF p : emLocs) {
        emLocsImg.append(QPoint(FP_TO_INT(p.x()), FP_TO_INT(p.y())));
    }

    // Fill in the image data
    Rgb2D_C pixArr(imgRect);

    // Image Generator
    ImageGen imgGen;
    imgGen.generateImage(pixArr, emLocsImg);

    if (0) {
        // Test gradient pattern
        for (int y = pixArr.yTop; y < pixArr.yTop + pixArr.height; y++) {
            for (int x = pixArr.xLeft; x < pixArr.xLeft + pixArr.width; x++) {
                pixArr.setPoint(x, y, qRgb(0, x * 255 / pixArr.width, y * 255 / pixArr.height));
            }
        }
    }

    RectToQString(this->rect());
    qDebug() << "Preview window is " << RectToQString(this->rect());
    qDebug() << "   View window is " << RectToQString(imgRect);

    QPainter painter(this);
    painter.setWindow(imgRect);

    QImage img((uchar*)pixArr.getDataPtr(), pixArr.width, pixArr.height, QImage::Format_ARGB32); // QRgb is ARGB32 (8 bits per channel)

    painter.drawImage(imgRect.x(), imgRect.y(), img);
}


