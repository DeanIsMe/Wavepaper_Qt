#include "imagegen.h"
#include <QList>
#include <QPoint>
#include <QElapsedTimer>


/** ****************************************************************************
 * @brief ImageGen::ImageGen
 */
ImageGen::ImageGen()
{



}

/** ****************************************************************************
 * @brief ImageGen::draw
 * @param width
 * @param height
 * @param pixData
 * @return
 */
int ImageGen::draw(int outWidth, int outHeight, QRgb * pixData) {
    QElapsedTimer fnTimer;
    fnTimer.start();

    QList<QPoint> emLocs;

    emLocs.append(QPoint(0.3, 0.5));
    emLocs.append(QPoint(0.5, 0.5));
    emLocs.append(QPoint(0.7, 0.5));

    if (emLocs.size() == 0) {
        qWarning("No emitters! Abort drawing");
        return 1;
    }

    // Generate a map of distance, depending on the offset
    int32_t distMapH = outHeight;
    int32_t distMapW = outWidth;
    double distDelta = 0.01;
    Double2D_C * distArr = new Double2D_C(distMapW, distMapH);
    calcDistArr(distDelta, *distArr);

    // Generate a map of the amplitudes
    double attnFactor = 1;
    Double2D_C * ampArr = new Double2D_C(distMapW, distMapH);
    calcAmpArr(attnFactor, *distArr, *ampArr);

    // Generate a map of the phasors
    double wavelength = 0.1;
    double distOffset = 12.5;
    Complex2D_C * phasorArr = new Complex2D_C(distMapW, distMapH);
    calcPhasorArr(wavelength, distOffset, *distArr, *ampArr, *phasorArr);

    // Sum phasor maps together


    // Use the resultant amplitude to fill in the pixel data
    for (int y = 0; y < outHeight; y++) {
        int hOff = y * outWidth;
        for (int x = 0; x < outWidth; x++) {
            pixData[hOff + x] = colourAngleToQrgb(std::abs(phasorArr->getPoint(x, y)) * 1530 * 5., 255); // For testing
        }
    }

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

    for (int32_t y = 0; y < arr.height; y++) {
        for (int32_t x = 0; x < arr.width; x++) {
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
    if (attnFactor == 0) {
        for (int32_t y = 0; y < ampArr.height; y++) {
            for (int32_t x = 0; x < ampArr.width; x++) {
                ampArr.setPoint(x, y, 1);
            }
        }
    }
    else if (attnFactor == 1) {
        for (int32_t y = 0; y < ampArr.height; y++) {
            for (int32_t x = 0; x < ampArr.width; x++) {
                ampArr.setPoint(x, y, 1/distArr.getPoint(x, y));
            }
        }
    }
    else {
        for (int32_t y = 0; y < ampArr.height; y++) {
            for (int32_t x = 0; x < ampArr.width; x++) {
                ampArr.setPoint(x, y, 1/pow(distArr.getPoint(x, y), attnFactor));
            }
        }
    }
    return;
}

/** ****************************************************************************
 * @brief ImageGen::calcP-hasorArr
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
 * @brief ImageGen::colourAngleToRGB8 sets a red, green and blue bytes to a point on a colour wheel
 * @param angle - ranges from 0 to 1529, representing the entire colour wheel
 * @return
 */
QRgb ImageGen::colourAngleToQrgb(int32_t angle, int alpha) {
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


