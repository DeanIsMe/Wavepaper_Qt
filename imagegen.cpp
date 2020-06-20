#include "imagegen.h"
#include <QList>
#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QWidget>

ImageGen imageGen;

/** ****************************************************************************
 * @brief ImageGen::ImageGen
 */
ImageGen::ImageGen()
{

}

/** ****************************************************************************
 * @brief ImageGen::drawPreview
 * @param targetWidget
 * @return
 */
int ImageGen::drawPreview(QWidget * targetWidget)
{
    qDebug("\n\nPaint preview");

    QRectF simWindow(0, 0, 100, 100. * 1920./1080.);
    simWindow.moveCenter(QPoint(0,0));

    // Determine the viewing window in image coordinates

    imgPerSimUnit = sqrt(targetImgPoints / simWindow.width() / simWindow.height());
    QRect imgRect(FP_TO_INT(simWindow.x() * imgPerSimUnit),
                  FP_TO_INT(simWindow.y() * imgPerSimUnit),
                  FP_TO_INT(simWindow.width() * imgPerSimUnit),
                  FP_TO_INT(simWindow.height()) * imgPerSimUnit);

    if (abs(simWindow.width() / simWindow.height() / (qreal)imgRect.width() * (qreal)imgRect.height() - 1) > 0.02) {
        qFatal("generateImage: viewWindow and simWindow are different ratios!");
        return -1;
    }

    QVector<EmitterI> emittersImg;
    if (PrepareEmitters(emittersImg)) {
        return -2;
    }

//    {
//        qDebug() << "Emitter locations (sim):";
//        QStringList strList;
//        for (EmitterF e : emittersF) {
//            strList.append(e.ToString());
//        }
//        qDebug(strList.join('\n').toLatin1());
//    }
    {
        qDebug() << "Emitter locations (img):";
        QStringList strList;
        for (EmitterI e : emittersImg) {
            strList.append(e.ToString());
        }
        qDebug(strList.join('\n').toLatin1());
    }


    qDebug() << "Simulation window is " << RectFToQString(simWindow) << "[sim units]";
    qDebug() << "Preview window is " << RectToQString(targetWidget->rect()) << "[real pixels]";
    qDebug() << "   View window is " << RectToQString(imgRect) << "[img units]";

    // Fill in the image data
    Rgb2D_C pixArr(imgRect);

    // Image Generator
    fillImageData(pixArr, emittersImg);

    if (0) {
        // Test gradient pattern
        for (int y = pixArr.yTop; y < pixArr.yTop + pixArr.height; y++) {
            for (int x = pixArr.xLeft; x < pixArr.xLeft + pixArr.width; x++) {
                pixArr.setPoint(x, y, qRgb(0, x * 255 / pixArr.width, y * 255 / pixArr.height));
            }
        }
    }

    QPainter painter(targetWidget);
    painter.setWindow(imgRect);

    QImage img((uchar*)pixArr.getDataPtr(), pixArr.width, pixArr.height, QImage::Format_ARGB32); // QRgb is ARGB32 (8 bits per channel)

    painter.drawImage(imgRect.x(), imgRect.y(), img);
    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::fillImageData
 * @param width
 * @param height
 * @param pixData
 * @return
 */
int ImageGen::fillImageData(Rgb2D_C & pixArr, QVector<EmitterI> & emitters) {
    QElapsedTimer fnTimer;
    fnTimer.start();

    // This function works in image logical coordinates, which are integers
    QRect imgRect = pixArr.getRect(); // The range that the result will be
    // generated for, in image coordinates

    if (emitters.size() == 0) {
        qWarning("fillImageData: No emitters! Abort drawing");
        return -1;
    }

    // Determine the range of the offset template
    QRect templateRange(0,0,0,0);
    for (EmitterI e : emitters) {
        templateRange |= imgRect.translated(-e.loc);
        // !@# need to upgrade the use of this template function to avoid crazy big arrays
    }
    qDebug() << "Template range is " << RectToQString(templateRange);

    // Generate a template array of distance, depending on the offset
    double simUnitPerIndex = 1 / imgPerSimUnit;
    Double2D_C * templateDist = new Double2D_C(templateRange);
    calcDistArr(simUnitPerIndex, *templateDist);

    // Generate a template array of the amplitudes
    Double2D_C * templateAmp = new Double2D_C(templateRange);
    calcAmpArr(s.attnFactor, *templateDist, *templateAmp);

    // Generate a map of the phasors for each emitter, and sum together
    // Use the templates
    double wlImg = s.wavelength * imgPerSimUnit; // Wavelength in image coordinates
    Complex2D_C * phasorSumArr = new Complex2D_C(imgRect);
    for (EmitterI e : emitters) {
        addPhasorArr(wlImg, e, *templateDist, *templateAmp, *phasorSumArr);
    }

    // Scaler will be 1/[emitter amplitude at half the simulation width]
    double scaler = pow(simUnitPerIndex * imgRect.width() / 2, s.attnFactor);

    // Use the resultant amplitude to fill in the pixel data
    for (int y = pixArr.yTop; y < pixArr.yTop + pixArr.height; y++) {
        for (int x = pixArr.xLeft; x < pixArr.xLeft + pixArr.width; x++) {
            pixArr.setPoint(x, y, colourAngleToQrgb(std::abs(phasorSumArr->getPoint(x, y)) * scaler * 1530 * 3., 255));
        }
    }

    delete templateDist;
    delete templateAmp;
    delete phasorSumArr;
    qDebug("ImageGen::draw took %4lld ms", fnTimer.elapsed());
    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::calcDistArr
 * @param simUnitPerIndex is the distance gap between each index
 * @param arr
 */
void ImageGen::calcDistArr(double simUnitPerIndex, Double2D_C & arr) {
    // Calculate the distance at every point

    for (int32_t y = arr.yTop; y < arr.yTop + arr.height; y++) {
        for (int32_t x = arr.xLeft; x < arr.xLeft + arr.width; x++) {
            arr.setPoint(x, y, simUnitPerIndex * sqrt((x*x) + (y*y)));
        }
    }
    return;
}


/** ****************************************************************************
 * @brief ImageGen::calcAmpArr
 * @param attnFactor is normally 1. Amplitude drops off at rate of 1/(r^attnFactor). 1/r is standard.
 * @param arr
 */
void ImageGen::calcAmpArr(double attnFactor, const Double2D_C & distArr, Double2D_C & ampArr) {
    // Calculate the complex phasor at every point
    if (distArr.getRect() != ampArr.getRect()) {
        qFatal("calcAmpArr distArr != ampArr! (not supported, but it could be)");
        return;
    }
    if (attnFactor == 0) {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1);
            }
        }
    }
    else if (attnFactor == 1) {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1/distArr.getPoint(x, y));
            }
        }
    }
    else {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, pow(distArr.getPoint(x, y), -attnFactor)); // 1/r^attnFactor = r^-attnFactor
            }
        }
    }
    return;
}

/** ****************************************************************************
 * @brief ImageGen::calcPhasorArr
 * @param wavelength
 * @param attnFactor is normally 1. Amplitude drops off at rate of 1/(r^attnFactor). 1/r is standard.
 * @param arr
 */
void ImageGen::calcPhasorArr(double wavelength, double distOffset,
                             const Double2D_C & distArr, const Double2D_C & ampArr, Complex2D_C & phasorArr) {
    // Calculate the complex phasor at every point
    double radPerDist = -2 * 3.14159265359 / wavelength; // Radians per unit distance, * -1
    for (int32_t y = 0; y < phasorArr.height; y++) {
        for (int32_t x = 0; x < phasorArr.width; x++) {
            phasorArr.setPoint(x, y,std::polar<fpComplex>(ampArr.getPoint(x,y), (distArr.getPoint(x, y) + distOffset) * radPerDist));
        }
    }
    return;
}

/** ****************************************************************************
 * @brief ImageGen::addPhasorArr for 1 emitter, calculates the phasor at every
 * location in the view window and add it to phasorArr
 * @param wavelength
 * @param distOffset causes a constant phase shift on every point
 * @param templateDist contains pre-calculated distance values for a given offset
 * @param templateAmp contains pre-calculated amplitude values for a given offset
 * @param emLoc is the emitter location that this phasor array is calculated for
 * @param phasorArr is the output array. It also defines the view window
 */
void ImageGen::addPhasorArr(double wavelength, EmitterI e,
                             const Double2D_C & templateDist, const Double2D_C & templateAmp,
                            Complex2D_C & phasorArr) {
    // phasorArray dimensions are that of the view window. emLoc is the location we're calculating for
    QRect rect = phasorArr.getRect().translated(-e.loc); // emitter location is the center
    // Distort the phasorArr coordinates during this function, such that the emitter is at the center
    phasorArr.translate(-e.loc);

    // Checks
    if (!templateDist.getRect().contains(phasorArr.getRect())) {
        qFatal("addPhasorArr - templateDist doesn't contain required offsets!");
        return;
    }
    if (!templateAmp.getRect().contains(phasorArr.getRect())) {
        qFatal("addPhasorArr - templateAmp doesn't contain required offsets!");
        return;
    }

    double radPerDist = -2 * 3.14159265359 / wavelength; // Radians per unit distance, * -1
    for (int32_t y = rect.top(); y < rect.top() + rect.height(); y++) {
        for (int32_t x = rect.x(); x < rect.x() + rect.width(); x++) {
            phasorArr.addPoint(x, y,std::polar<fpComplex>(templateAmp.getPoint(x,y),
                                                          (templateDist.getPoint(x, y) + e.distOffset) * radPerDist));
        }
    }
    // Restore the phasorArr coordinates
    phasorArr.translate(e.loc);
    return;
}


/** ****************************************************************************
 * @brief ImageGen::colourAngleToQrgb sets a red, green and blue bytes to a point on a colour wheel
 *        This is useful for a simple colour map
 * @param angle - ranges from 0 to 1529, representing the entire colour wheel
 * @param alpha is the alpha value - just passed on to the colour
 * @return
 */
QRgb ImageGen::colourAngleToQrgb(int32_t angle, uint8_t alpha) {
    angle = angle % 1530; // 1530 steps/values total.
    int32_t stage = angle/255; // 0 to 5
    uint8_t colour[3];
    // Set the starting levels for this stage
    // for (u8 i=0; i<3; i++) {*colour[i] = ((stage+2*(i+1))%6)/3 ? 255 : 0;} // Loop version
    colour[0] = ((stage+2)%6)/3 ? 255 : 0;
    colour[1] = ((stage+4)%6)/3 ? 255 : 0;
    colour[2] = ((stage+6)%6)/3 ? 255 : 0;
    colour[stage%3] += stage%2 ? -(angle%255) : angle%255;

    return qRgba(colour[0], colour[1], colour[2], alpha);
}


/** ****************************************************************************
 * @brief emitterArrangementToLocs determines and returns the locations of the emitters, given the arrangement
 * @param arngmt
 * @param emLocsOut
 * @return
 */
int ImageGen::emitterArrangementToLocs(const EmArrangement & arngmt, QVector<QPointF> & emLocsOut) {
    switch (arngmt.type) {
    case EmType::blank:
        emLocsOut.resize(0);
        break;
    case EmType::arc:
        emLocsOut.resize(arngmt.count);
        for (int32_t i = 0; i < emLocsOut.size(); i++) {
            // First, calc the angle from +ve x to start at
            double angle = 3 * 3.1415926/2 + arngmt.arcAng * ((i-0.5)/arngmt.count - 0.5);
            emLocsOut[i] = QPointF(arngmt.arcRadius * cos(angle), arngmt.arcRadius * sin(angle));
        }
        break;
    case EmType::line:
        emLocsOut.resize(arngmt.count);
        if (arngmt.count == 1) {
            emLocsOut[0] = QPointF(0, 0);
        }
        else {
            double step = 1.0/(arngmt.count-1.0);
            for (int32_t i = 0; i < emLocsOut.size(); i++) {
                emLocsOut[i] = QPointF(arngmt.lenTotal * (-0.5 + i * step), 0);
            }
        }
        break;
    case EmType::square:
        // !@#$ implement!
        break;
    case EmType::custom:
        emLocsOut = arngmt.customLocs;
        break;
    }

    // Rotate and translate the scatterers, if needed (about the origin)
    QTransform transform;
    transform.rotate(-arngmt.rotation);
    transform.translate(arngmt.center.x(), arngmt.center.y());
    for (QPointF &p : emLocsOut) {
        p = transform.map(p)   ;
    }

    // Mirror
    if (arngmt.mirrorHor) {
        int32_t len = emLocsOut.size();
        emLocsOut.resize(len * 2);
        QTransform mirror(-1., 0., 0., 1., 0, 0);
        for (int32_t i = 0; i < len/2; i++) {
            emLocsOut[len + i] = mirror.map(emLocsOut[i]);
        }
    }
    if (arngmt.mirrorVert) {
        int32_t len = emLocsOut.size();
        emLocsOut.resize(len * 2);
        QTransform mirror(1., 0., 0., -1., 0, 0);
        for (int32_t i = 0; i < len/2; i++) {
            emLocsOut[len + i] = mirror.map(emLocsOut[i]);
        }
    }
    return 0;
}


/** ****************************************************************************
 * @brief PrepareEmitters
 * @param emittersImg
 */
int ImageGen::PrepareEmitters(QVector<EmitterI> emittersImg) {

    // Emitter locations
    EmArrangement arn;
    arn.type = EmType::arc;
    arn.arcRadius = 30;
    arn.arcAng = 90;
    arn.count = 5;

    QVector<QPointF> emLocs;
    emitterArrangementToLocs(arn, emLocs);
    QVector<EmitterF> emittersF(emLocs.size());
    for (int32_t i = 0; i < emLocs.size(); i++) {
        emittersF[i].loc = emLocs[i];
    }

    /*
    QList<EmitterF> emittersF;
    emittersF.append(EmitterF(QPointF(-30, 0)));
    emittersF.append(EmitterF(QPointF(0, 0)));
    emittersF.append(EmitterF(QPointF(30, 0)));
    */

    if (emittersF.size() == 0) {
        qWarning("No emitters! Abort drawing");
        return -2;
    }

    // Convert emLocs to image coordinates
    emittersImg.resize(emittersF.size());
    for (int32_t i = 0; i < emLocs.size(); i++) {
        emittersImg[i] = EmitterI(emittersF[i], imgPerSimUnit);
    }
    return 0;
}
