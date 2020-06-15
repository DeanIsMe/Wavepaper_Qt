#include "imagegen.h"
#include <QList>
#include <QElapsedTimer>


/** ****************************************************************************
 * @brief ImageGen::ImageGen
 */
ImageGen::ImageGen()
{

}

/** ****************************************************************************
 * @brief ImageGen::generateImage
 * @param width
 * @param height
 * @param pixData
 * @return
 */
int ImageGen::generateImage(Rgb2D_C & pixArr) {
    QElapsedTimer fnTimer;
    fnTimer.start();

    QRect viewWindow = pixArr.getRect();

    QList<QPoint> emLocs;
    emLocs.append(QPoint(30, 50));
    emLocs.append(QPoint(50, 50));
    emLocs.append(QPoint(70, 50));

    if (emLocs.size() == 0) {
        qWarning("No emitters! Abort drawing");
        return 1;
    }

    // Determine the range of the offset template
    QRect templateRange(0,0,0,0);
    for (QPoint p : emLocs) {
        templateRange |= viewWindow.translated(-p);
        // !@# need to upgrade the use of this template function to avoid crazy big arrays
    }
    qDebug("Template range: @(%d, %d), %d x %d",
           templateRange.x(), templateRange.y(),
           templateRange.width(), templateRange.height());
    qDebug("ViewWindow : @(%d, %d), %d x %d",
           viewWindow.x(), viewWindow.y(),
           viewWindow.width(), viewWindow.height());

    // Generate a template array of distance, depending on the offset
    double distDelta = 0.01;
    Double2D_C * templateDist = new Double2D_C(templateRange);
    calcDistArr(distDelta, *templateDist);

    // Generate a template array of the amplitudes
    double attnFactor = 1;
    Double2D_C * templateAmp = new Double2D_C(templateRange);
    calcAmpArr(attnFactor, *templateDist, *templateAmp);

    // Generate a map of the phasors for each emitter, and sum together
    // Use the templates
    double wavelength = 0.3;
    Complex2D_C * phasorSumArr = new Complex2D_C(viewWindow);
    for (QPoint p : emLocs) {
        double distOffset = 12.5;
        addPhasorArr(wavelength, distOffset, *templateDist, *templateAmp, p, *phasorSumArr);
    }

    // Use the resultant amplitude to fill in the pixel data
    for (int y = pixArr.yTop; y < pixArr.yTop + pixArr.height; y++) {
        for (int x = pixArr.xLeft; x < pixArr.xLeft + pixArr.width; x++) {
            pixArr.setPoint(x, y, colourAngleToQrgb(std::abs(phasorSumArr->getPoint(x, y)) * 1530 * 5., 255));
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
 * @param delta is the distance gap between each index
 * @param arr
 */
void ImageGen::calcDistArr(double delta, Double2D_C & arr) {
    // Calculate the distance at every point

    for (int32_t y = arr.yTop; y < arr.yTop + arr.height; y++) {
        for (int32_t x = arr.xLeft; x < arr.xLeft + arr.width; x++) {
            arr.setPoint(x, y, delta * sqrt((x*x) + (y*y)));
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
                ampArr.setPoint(x, y, 1/pow(distArr.getPoint(x, y), attnFactor));
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
void ImageGen::addPhasorArr(double wavelength, double distOffset,
                             const Double2D_C & templateDist, const Double2D_C & templateAmp,
                            QPoint emLoc, Complex2D_C & phasorArr) {
    // phasorArray dimensions are that of the view window. emLoc is the location we're calculating for
    QRect rect = phasorArr.getRect().translated(-emLoc); // emitter location is the center
    // Distort the phasorArr coordinates during this function, such that the emitter is at the center
    phasorArr.translate(-emLoc);

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
                                                          (templateDist.getPoint(x, y) + distOffset) * radPerDist));
        }
    }
    // Restore the phasorArr coordinates
    phasorArr.translate(emLoc);
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


