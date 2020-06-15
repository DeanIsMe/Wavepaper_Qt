#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QPoint>
#include <QRect>
#include "datatypes.h"

/** ****************************************************************************
 * @brief The ImageGen class
 */
class ImageGen
{
public:
    ImageGen();
    int generateImage(Rgb2D_C &pixArr, QList<QPoint> &emLocsImg);
private:
    static void calcDistArr(double simUnitPerIndex, Double2D_C &arr);
    static void calcAmpArr(double attnFactor, const Double2D_C &distArr, Double2D_C &ampArr);
    static void calcPhasorArr(double wavelength, double distOffset, const Double2D_C &distArr, const Double2D_C &ampArr, Complex2D_C &phasorArr);
    static QRgb colourAngleToQrgb(int32_t angle, uint8_t alpha = 255);
    static void addPhasorArr(double wavelength, double distOffset, const Double2D_C &templateDist, const Double2D_C &templateAmp, QPoint emLoc, Complex2D_C &phasorArr);
};

#endif // IMAGEGEN_H
