#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QRgb>
#include <complex>
#include <QPoint>
#include <QRect>

/** ****************************************************************************
 * @brief The Map2D_C class
 * A 2D array of a data type
 */
template <typename T>
class Array2D_C {

private:
    T* data = nullptr;
    T* dataZero; // Pointer that corresponds to the index [0,0]. It may not be a valid address
public:
    int32_t xLeft; // Usually negative
    int32_t yTop; // Usually negative
    const int32_t width;
    const int32_t height;

public:
    explicit Array2D_C(int32_t xLeft_, int32_t yTop_, int32_t width_, int32_t height_) :
        xLeft(xLeft_), yTop(yTop_),
        width(width_), height(height_) {
        data = new T[width * height];
        dataZero = &data[-yTop * width - xLeft];
    }
    explicit Array2D_C(QPoint topLeft, QSize sz) :
        Array2D_C(topLeft.x(), topLeft.y(), sz.width(), sz.height()) {}
    explicit Array2D_C(QRect rect) :
        Array2D_C(rect.topLeft(), rect.size()) {}

    ~Array2D_C() {
        if (data != nullptr) {
            delete data;
        }
    }

    T* getDataPtr() const {return data;}

    void translate(int32_t deltaX, int32_t deltaY) {
        xLeft += deltaX;
        yTop += deltaY;
        dataZero = &dataZero[-deltaY * width - deltaX];
    }
    inline void translate(QPoint p) {translate(p.x(), p.y());}

    inline T& getPoint(int32_t x, int32_t y) const {
        return dataZero[x + y * width];
    }
    inline void setPoint(int32_t x, int32_t y, const T& val) const {
        dataZero[x +  + y * width] = val;
    }
    inline void addPoint(int32_t x, int32_t y, const T& val) const {
        dataZero[x +  + y * width] += val;
    }
    inline T& getPoint(QPoint p) const {
        return dataZero[p.x() + p.y() * width];
    }
    inline void setPoint(QPoint p, const T& val) const {
        dataZero[p.x() + p.y() * width] = val;
    }
    inline void addPoint(QPoint p, const T& val) const {
        dataZero[p.x() + p.y() * width] += val;
    }
    QRect getRect() const {return QRect(xLeft, yTop, width, height);}
};

template <typename T>
class Array2DMap_C {
private:
    T* data = nullptr;
public:
    const int32_t xLeft; // Usually negative
    const int32_t yTop; // Usually negative
    const int32_t width;
    const int32_t height;

    explicit Array2DMap_C(int32_t xLeft_, int32_t xRight_, int32_t yTop_, int32_t yBottom_) :
        width(xRight_ - xLeft_), height(yBottom_ - yTop_),
        xLeft(xLeft_), yTop(yTop_) {
        data = new T[width * height]();
    }
    ~Array2DMap_C() {
        if (data != nullptr) {
            delete data;
        }
    }


    inline T& getPoint(int32_t x, int32_t y) const {
        return data[x - xLeft + (y - yTop) * width];
    }
    inline void setPoint(int32_t x, int32_t y, const T& val) {
        data[(x - xLeft) +  + (y - yTop) * width] = val;
    }
    inline T& getPoint(QPoint p) const {
        return data[(p.x() - xLeft) + (p.y() - yTop) * width];
    }
    inline void setPoint(QPoint p, const T& val) {
        data[(p.x() - xLeft) + (p.y() - yTop) * width] = val;
    }
};

class CoordMapper_C {
    // Maps Input coordinates (including negatives) to array indices
public:
    const int32_t xLeft; // Usually negative
    const int32_t yTop; // Usually negatifve
    inline QPoint coordToIndices(int32_t x, int32_t y) {
        return QPoint(x - xLeft, y - yTop);
    }
    inline int32_t xToIndex(int32_t x) {
        return x - xLeft;
    }
    inline int32_t yToIndex(int32_t y) {
        return y - yTop;
    }
};


typedef double fpComplex; // Float or double
typedef std::complex<fpComplex> complex;
typedef Array2D_C<complex> Complex2D_C;
typedef Array2D_C<double> Double2D_C;
typedef Array2D_C<QRgb> Rgb2D_C;

/** ****************************************************************************
 * @brief The ImageGen class
 */
class ImageGen
{
public:
    ImageGen();
    int generateImage(Rgb2D_C &pixArr);
private:
    static void calcDistArr(double delta, Double2D_C &arr);
    static void calcAmpArr(double attnFactor, const Double2D_C &distArr, Double2D_C &ampArr);
    static void calcPhasorArr(double wavelength, double distOffset, const Double2D_C &distArr, const Double2D_C &ampArr, Complex2D_C &phasorArr);
    static QRgb colourAngleToQrgb(int32_t angle, uint8_t alpha = 255);
    static void addPhasorArr(double wavelength, double distOffset, const Double2D_C &templateDist, const Double2D_C &templateAmp, QPoint emLoc, Complex2D_C &phasorArr);
};

#endif // IMAGEGEN_H
