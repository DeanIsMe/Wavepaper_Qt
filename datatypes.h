#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QStringBuilder>
#include <QRect>
#include <QRgb>
#include <QDebug>
#include <complex>

#define FP_TO_INT(fp) (fp + 0.5 - (fp<0))
#define PI 3.1415926

class ImageGen;
class PreviewView;
class PreviewScene;

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

typedef qreal fpComplex; // Float or double
typedef std::complex<fpComplex> complex;
typedef Array2D_C<complex> Complex2D_C;
typedef Array2D_C<double> Double2D_C;
typedef Array2D_C<QRgb> Rgb2D_C;

/** ****************************************************************************
 * @brief The State_S struct
 */
// !@#$ delete me
struct State_S {

};
extern State_S state;


inline QString RectToQString(const QRect & r) {
    return QString::asprintf("%d x %d @(%d, %d)", r.width(), r.height(), r.x(), r.y());
}

inline QString RectFToQString(const QRectF & r) {
    return QString::asprintf("%.1f x %.1f @(%.1f, %.1f)", r.width(), r.height(), r.x(), r.y());
}


/**
 * modulus operation, that always returns a positive number.
 * e.g. mod(-1, 7) = 6. With %, the result would be -1.
 */
inline qint32 modPos(qint32 a, qint32 b) {
   if(b < 0) //you can check for b == 0 separately and do what you want
     return modPos(-a, -b);
   qint32 ret = a % b;
   if(ret < 0)
     ret+=b;
   return ret;
}

inline qreal modPos(qreal a, qreal b) {
   if(b < 0) //you can check for b == 0 separately and do what you want
     return modPos(-a, -b);
   qreal ret = fmod(a, b);
   if(ret < 0)
     ret+=b;
   return ret;
}

qreal Snap(qreal val, qreal snapInc, qreal snapWithin);
qreal Snap(qreal val, qreal snapInc);
qint32 Snap(qint32 val, qint32 snapInc, qint32 snapWithin);


#endif // DATATYPES_H
