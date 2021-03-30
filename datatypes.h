#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>
#include <QStringBuilder>
#include <QRect>
#include <QRgb>
#include <QDebug>
#include <QColor>
#include <QPainterPath>
#include <complex>

#define FP_TO_INT(fp) (fp + 0.5 - (fp<0))
#define PI (3.14159265359)

class MainWindow;
class ImageGen;
class PreviewView;
class PreviewScene;

enum class ProgramMode {
    waves,
    fourBar,
};

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



/** ************************************************************************ **/
struct TemplateDist {
    Double2D_C * arr = nullptr; // Index is image units. Values are scene units
    qreal imgPerSimUnit; // The imgPerSimUnit that this template was generated with
};
struct TemplateAmp {
    Double2D_C * arr = nullptr; // Index is image units
    qreal distOffset; // the distOffset used in the template amp calculation
};
struct TemplatePhasor {
    // Use the MakeNew function to make any changes to these variables (ensures that variables are in sync)
    Complex2D_C * arr = nullptr; // Simulation units. Depends on imgPerSimUnit or wavelength.
    qreal wavelength; // The wavelength that this template was generated with
    qreal imgPerSimUnit; // The imgPerSimUnit that this template was generated with
    void MakeNew(QRect size, qreal wavelengthIn, qreal imgPerSimUnitIn);
};
struct SumArray {
    Complex2D_C * phasorArr = nullptr; // Resultant phasor of all emitters summed together
    Double2D_C * ampArr = nullptr; // Amplitude of each point in sumArr
    double ampMax = 0;
    double ampMin = 999999;
    qint32 checkSum = 0;
};

/** ****************************************************************************
 * @brief The GenSettings struct
 */
struct GenSettings {
    static constexpr qint32 dfltImgPointsQuick = 500000; // Target pixels in the image
    static constexpr qint32 dfltImgPointsPreview = 1000000; // Target pixels in the image
    double targetImgPoints = dfltImgPointsPreview; // Total number of points in the preview. Change with setTargetImgPoints()
    double imgPerSimUnit; // The imgPerSimUnit that this template was generated with
    QRect areaImg; // The rectangle of the image view area (image coordinates)
    bool indexedClr = true; // True for faster (but less accurate) colour map
    // Cached data
    TemplateDist templateDist;
    TemplateAmp templateAmp;
    TemplatePhasor templatePhasor;
    SumArray combinedArr;
    // Colour index
    int clrIndexMax = 200; // The colour indices span from 0 to this number
    // Colour map
    qint32 clrListCheckSum = 0; // The checksum for the colour list when the index was generated
    QVector<QColor> clrIndexed; // All colours from locations 0 to 1.0 (indices 0 to clrIndexMax)
    QVector<QRgb> clrIndexedRgb; // All colours from locations 0 to 1.0 (indices 0 to clrIndexMax)
    // Mask map
    qint32 maskCheckSum = 0; // The checksum for the mask when the index was generated
    QVector<qreal> maskIndexed; // All mask values from locations 0 to 1.0 (indices 0 to clrIndexMax). Values are 0 to 1.0.
    QVector<quint32> maskIndexedInt; // All mask values from locations 0 to 1.0 (indices 0 to clrIndexMax). Values are (0 to 255) << 24

    // Four bar linkage
    QPainterPath paintPath;
    qreal pointsPerRev; // Setting. How many points in the path per revolution of A & B. Low = polygonal. High = quality curves. 100 is low quality. 200 = high quality.
};


/** ****************************************************************************
 * @brief The ClrMapPreset enum lists the available preset types
 * For any added type, a button should be added.
 */
enum class ClrMapPreset {
    hot,
    cool,
    jet,
    bone,
    parula,
    hsv,
};

/** ****************************************************************************
 * @brief The ClrFix struct is just a combination of colour + location (0 to 1)
 */
struct ClrFix {
    QColor clr; // Colour at this location
    qreal loc; // 0.0 to 1.0
    ClrFix(QColor _clr, qreal _loc) : clr(_clr), loc(_loc) {}
    bool operator <(const ClrFix& other) const {
        return loc < other.loc;
    }
    bool operator >(const ClrFix& other) const {
        return loc > other.loc;
    }
    bool operator==(const ClrFix& other) const {
        return loc == other.loc;
    }
};

typedef QVector<ClrFix> ColourList;

/** ****************************************************************************
 * @brief The MaskCfg struct
 */
struct MaskCfg { // Mask settings
    bool enabled = false; // True if the mask is on
    qreal numRevs = 3; // How many ripples from from min to max
    qreal offset = 0; // Phase offset, as a count of periods. Should be 0 to 1.
    qreal dutyCycle = 0.3; // 0.5 for even (50%). Must be 0 to 1.0
    qreal smooth = 0.5; // Width factor of transition. 0=immediate transition. 1.0=transition is 50% of period.
    QColor backColour = QColor(0,0,0); // When mask is 0%, what the colour will be
};


/** ****************************************************************************
 * @brief The FourBarCfg struct holds all of the settings that define the four
 * bar linkage
 */
struct FourBarCfg {
    qreal baseSepX = 0.6; // Horizontal separation of the 2 bases are, as a factor of lenBase. 0+
    qreal baseOffsetY = 0.; // Vertical offset of the 2 bases, as a factor of lenBase; -ve to +ve

    qreal lenRatioB = 1.0; // Length of Arm B = Length of Arm A * lenRatioB;
    qreal lenRatio2 = 4.0; // Length of Segment 2 = Length of Segment 1 * lenRatio2;
    qreal lenBase = 30;

    qreal ta1Init = 0; // Initial angle of a1 [rad]
    qreal initAngleOffset = 0; // initAngleB = initAngleA + initAngleOffset [rad]
    qreal revCount = 20; // When to stop drawing, in number of revolutions (A + B combined)

    qreal revRatioB = 1.02; // The rate of increasing the angle of 'B' vs 'A'

    qreal lineWidth = 1.0;
    qreal lineTaperRatio = 0.8;
    qreal temp = 0.01; // !@#
};


/** ****************************************************************************
 * @brief The Settings struct holds all of the settings that describe the
 * current pattern
 */
struct Settings {
    double wavelength = 40; // Wavelength. Simulation units
    double distOffsetF = 0.1; // controls linearity. Range 0 to 1+, normally 0.1. Amplitude drops off at rate of 1/(r + sceneLength * distOffsetF).
    // as distOffsetF approaches 0, the amplitude at each emitter approaches infinity.
    bool emittersInSync; // If true then all emitters are in phase with the same amplitude. If false, then the energizer determines phase & amplitude
    QPointF energizerLoc; // The location of the energizer that determines amplitude and phase by the distance to each emitter
    QList<EmArrangement> emArrangements;
    ColourList clrList; // Editing this should be handled through the ColourMap class, to update the table accordingly
    MaskCfg maskCfg;
    struct {
        double aspectRatio = 1.; // width / height of the preview window, simulation area, generated image, ...
        double zoomLevel = 1; //
        QPointF center{0,0};
    } view;
    // Controlling information displayed
    // (Doesn't affect the actual patten)
    double emitterRadius = 2.; // Emitter radius, simulation units

    // Four bar linkage
    FourBarCfg fourBar;
};



#endif // DATATYPES_H
