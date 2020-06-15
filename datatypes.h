#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QRect>
#include <QRgb>
#include <QDebug>
#include <complex>

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

typedef double fpComplex; // Float or double
typedef std::complex<fpComplex> complex;
typedef Array2D_C<complex> Complex2D_C;
typedef Array2D_C<double> Double2D_C;
typedef Array2D_C<QRgb> Rgb2D_C;


inline QString RectToQString(const QRect & r) {
    return QString::asprintf("%d x %d @(%d, %d)", r.width(), r.height(), r.x(), r.y());
}


#endif // DATATYPES_H
