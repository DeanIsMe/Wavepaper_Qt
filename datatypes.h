#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QStringBuilder>
#include <QRect>
#include <QRgb>
#include <QDebug>
#include <complex>

#define FP_TO_INT(fp) (fp + 0.5 - (fp<0))
#define PI (3.14159265359)

class MainWindow;
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
    T* dataZero = nullptr; // Pointer that corresponds to the index [0,0]. It may not be a valid address
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
    QRect rect() const {return QRect(xLeft, yTop, width, height);}
};

typedef qreal fpComplex; // Float or double
typedef std::complex<fpComplex> complex;
typedef Array2D_C<complex> Complex2D_C;
typedef Array2D_C<double> Double2D_C;
typedef Array2D_C<QRgb> Rgb2D_C;

/** ************************************************************************ **/
/// Emitters

/**
 * @brief The EmType enum
 */
enum class EmType {
    // Emitter arrangement types
    blank,
    arc,
    square,
    line,
    custom,
};

struct EmArrangement {
    EmType type = EmType::blank;
    QPointF center = QPointF(0,0);
    int count = 1;
    double rotation = 0; // in radians
    bool mirrorHor = false;
    bool mirrorVert = false;

    // Circular specific
    double arcRadius = 20;
    double arcSpan = 1.5708; // Radians
    // Linear specific
    double lenTotal = 40; // The total length of the line
    QVector<QPointF> customLocs;
};

struct EmitterF { // Emitter, floating point coords
    QPointF loc; // Simulation coordinates
    double distOffset; // determines the phase. default 0. NOT SUPPORTED (for efficiency)!
    double amplitude; // default of 1
    EmitterF() : loc(0,0), distOffset(0), amplitude(1) {}
    EmitterF(QPointF p) : loc(p), distOffset(0), amplitude(1) {}
    EmitterF(QPointF p, double distOffset_, double amp) : loc(p), distOffset(distOffset_), amplitude(amp) {}
    QString ToString() {return QString::asprintf("Em_f @(%5.1f, %5.1f). o=%.1f. a=%.2f.",
                                                 loc.x(), loc.y(), distOffset, amplitude);}
};

struct EmitterI { // Emitter, integer coords
    QPoint loc; // Image coordinates (integers)
    double distOffset; // determines the phase. NOT SUPPORTED!
    double amplitude; // default of 1
    EmitterI() : loc(0,0), distOffset(0), amplitude(1) {}
    EmitterI(QPoint p) : loc(p), distOffset(0), amplitude(1) {}
    EmitterI(QPoint p, double distOffset_, double amp) : loc(p), distOffset(distOffset_), amplitude(amp) {}
    EmitterI(EmitterF e, double imgPerSimUnit) :
        loc((e.loc * imgPerSimUnit).toPoint()),
                              distOffset(e.distOffset), amplitude(e.amplitude) {}
    QString ToString() {return QString::asprintf("Em_I @(%4d, %4d). o=%.1f. a=%.2f.",
                                                 loc.x(), loc.y(), distOffset, amplitude);}
};

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/

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

/** ****************************************************************************
 * @brief The CheckSum class performs a very simple sum over data
 * ranges, treating all data as arrays of uint8_t. Used to compare data.
 */
class CheckSum {
public:
    qint32 sum = 0;
    CheckSum() {}
    CheckSum(void* ptr, int byteCount) {
        Add(ptr, byteCount);
    }
    inline void Add(quint8 val) {
        sum += val;
    }
    void Add(void* ptr, int byteCount) {
        quint8 * ptr8 = (quint8 *) ptr;
        quint8 * ptrEnd = ptr8 + byteCount;
        while (ptr8 < ptrEnd) {
            Add(*ptr8++);
        }
    }
    void Add(qint8 val) {
        return Add((quint8)val);
    }
    void Add(qint16 val) {
        return Add((void *) &val, sizeof(val));
    }
    void Add(quint16 val) {
        return Add((void *) &val, sizeof(val));
    }
    void Add(qint32 val) {
        return Add((void *) &val, sizeof(val));
    }
    void Add(quint32 val) {
        return Add((void *) &val, sizeof(val));
    }
    qint32 Get() {return sum;}
    operator ==(CheckSum b) {return sum == b.sum;}
    operator !=(CheckSum b) {return sum != b.sum;}
    operator int() const {return sum;}
};


#endif // DATATYPES_H
