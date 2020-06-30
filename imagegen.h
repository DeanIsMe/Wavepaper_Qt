#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QPoint>
#include <QRect>
#include <QList>
#include <QPainter>
#include <QGraphicsView>
#include "datatypes.h"

enum class EmType {
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
    double arcAng = 1.5708;
    // Linear specific
    double lenTotal = 40; // The total length of the line
    QVector<QPointF> customLocs;
};

struct EmitterF { // Emitter, floating point coords
    QPointF loc; // Simulation coordinates
    double distOffset; // determines the phase. default 0
    double amplitude; // default of 1
    EmitterF() : loc(0,0), distOffset(0), amplitude(1) {}
    EmitterF(QPointF p) : loc(p), distOffset(0), amplitude(1) {}
    EmitterF(QPointF p, double distOffset_, double amp) : loc(p), distOffset(distOffset_), amplitude(amp) {}
    QString ToString() {return QString::asprintf("Em_f @(%5.1f, %5.1f). o=%.1f. a=%.2f.",
                                                 loc.x(), loc.y(), distOffset, amplitude);}
};

struct EmitterI { // Emitter, integer coords
    QPoint loc; // Image coordinates (integers)
    double distOffset; // determines the phase
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

struct Settings {
    double wavelength = 40; // Wavelength. Simulation units
    double attnFactor = 1; // Normally 1. Amplitude drops off at rate of 1/(r^attnFactor). 1/r is standard.
    bool emittersInSync; // If true then all emitters are in phase with the same amplitude. If false, then the energizer determines phase & amplitude
    QPointF energizerLoc; // The location of the energizer that determines amplitude and phase by the distance to each emitter
    QList<EmArrangement> emArrangements;
    struct {
        double aspectRatio = 1080./1920.; // width / height of the preview window, simulation area, generated image, ...
        double zoomLevel = 1; //
        QPointF center{0,0};
    } view;
    // Controlling information displayed
    double emitterRadius = 2.; // Emitter radius, simulation units

};

/** ****************************************************************************
 * @brief The ImageGen class
 */
class ImageGen
{
private:
    QList<EmArrangement> arngmtList;
    Double2D_C * templateDist = nullptr;
    Double2D_C * templateAmp = nullptr;

public:
    Settings s;

    // The block below must be kept in sync
    double targetImgPoints = 100000; // Total number of points in the preview
    double imgPerSimUnit;
    QRectF simArea; // The rectangle of the image view area (simulation coordinates)
    QRect imgArea; // The rectangle of the image view area (image coordinates)
    QSize outResolution; // The output will be rendered to this resolution

    qreal aspectRatio() const {return (qreal)outResolution.width() / (qreal)outResolution.height();} // Width / height
    QImage image;
    qreal testVal = 1;

public:
    ImageGen();
    int GenerateImage(QImage &imageOut);
    EmArrangement* GetActiveArrangement();
    int InitViewAreas();
private:
    static void CalcDistArr(double simUnitPerIndex, Double2D_C &arr);
    static void CalcAmpArr(double attnFactor, const Double2D_C &distArr, Double2D_C &ampArr);
    static void CalcPhasorArr(double wavelength, double distOffset, const Double2D_C &distArr, const Double2D_C &ampArr, Complex2D_C &phasorArr);
    static QRgb ColourAngleToQrgb(int32_t angle, uint8_t alpha = 255);
    static void AddPhasorArr(double imgPerSimUnit, double wavelength, EmitterI e, const Double2D_C &templateDist, const Double2D_C &templateAmp, Complex2D_C &phasorArr);
    static int EmitterArrangementToLocs(const EmArrangement &arngmt, QVector<QPointF> &emLocsOut);
    void CalcTemplates(QRect templateRect);

public:
    int GetEmitterList(QVector<EmitterF> &emitters);
    static void DebugEmitterLocs(const QVector<EmitterI>& emittersImg);
    static void DebugEmitterLocs(const QVector<EmitterF> &emittersF);
    static EmArrangement DefaultArrangement();
    void setTargetImgPoints(qint32 imgPoints);
};

extern ImageGen imageGen;

#endif // IMAGEGEN_H
