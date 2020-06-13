#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QRgb>
#include <complex>
#include <QPoint>

/** ****************************************************************************
 * @brief The Map2D_C class
 * A 2D array of complex numbers
 */
template <typename T>
class Array2D_C {

private:
     T* data = nullptr;
public:
    const int32_t width;
    const int32_t height;

public:
    explicit Array2D_C(int32_t width_, int32_t height_) : width(width_), height(height_) {
        data = new T[width * height];
    }
    ~Array2D_C() {
        if (data != nullptr) {
            delete data;
        }
    }

    inline T& getPoint(int32_t x, int32_t y) const {
        return data[x + y * width];
    }
    inline void setPoint(int32_t x, int32_t y, const T& val) {
        data[x + y * width] = val;
    }
    inline T& getPoint(QPoint p) const {
        return data[p.x() + p.y() * width];
    }
    inline void setPoint(QPoint p, const T& val) {
        data[p.x() + p.y() * width] = val;
    }
};

typedef double fpComplex; // Float or double
typedef std::complex<fpComplex> complex;
typedef Array2D_C<complex> Complex2D_C;
typedef Array2D_C<double> Double2D_C;

/** ****************************************************************************
 * @brief The ImageGen class
 */
class ImageGen
{
public:
    ImageGen();
    int draw(int outWidth, int outHeight, QRgb * pixData);
private:
    static void calcDistArr(double delta, Double2D_C &arr);
    static void calcAmpArr(double attnFactor, const Double2D_C &distArr, Double2D_C &ampArr);
    static void calcPhasorArr(double wavelength, double distOffset, const Double2D_C &distArr, const Double2D_C &ampArr, Complex2D_C &phasorArr);
    static QRgb colourAngleToQrgb(int32_t angle, int alpha);
};

#endif // IMAGEGEN_H
