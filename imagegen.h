#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QObject>
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

struct Settings {
    double wavelength = 40; // Wavelength. Simulation units
    double distOffsetF = 0.1; // controls linearity. Range 0 to 1+, normally 0.1. Amplitude drops off at rate of 1/(r + sceneLength * distOffsetF).
    // as distOffsetF approaches 0, the amplitude at each emitter approaches infinity.
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
class ImageGen : public QObject
{
    Q_OBJECT
public:
    // PROGRAM SETTINGS
    static constexpr qint32 imgPointsQuick = 40000;
    static constexpr qint32 imgPointsPreview = 200000;
    static constexpr qreal templateOversizeFactor = 1.2; // The amount of extra length that the templates are calculated for

    class Interact {
    public:
        Interact(ImageGen * parentIn) : parent(parentIn) {}
        void mousePressEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
        bool IsActive() {return active;}
        EmArrangement * GetActiveGroup() { return grpActive; }


    private:
        void Cancel();
        ImageGen * const parent;
        bool active = false;
        EmArrangement grpBackup;
        EmArrangement * grpActive;
        QPointF pressPos;
        bool ctrlPressed;
    } act;


private:
    MainWindow * mainWindow = nullptr;
    QList<EmArrangement> arngmtList;
    struct TemplateDist {
        Double2D_C * arr = nullptr; // Index is image units. Values are scene units
        qreal imgPerSimUnit; // The imgPerSimUnit that this template was generated with
    };
    struct TemplateAmp {
        Double2D_C * arr = nullptr; // Index is image units
        qreal distOffset; // the distOffset used in the template amp calculation
    };
    struct TemplatePhasor {
        Complex2D_C * arr = nullptr; // Simulation units. Depends on imgPerSimUnit or wavelength.
        qreal wavelength; // The wavelength that this template was generated with
        qreal imgPerSimUnit; // The imgPerSimUnit that this template was generated with
        void MakeNew(QRect size, qreal wavelengthIn, qreal imgPerSimUnitIn);
    };

public:
    Settings s;

    struct GenSettings {
        double targetImgPoints = imgPointsPreview; // Total number of points in the preview
        double imgPerSimUnit;
        QRect areaImg; // The rectangle of the image view area (image coordinates)
        TemplateDist templateDist;
        TemplateAmp templateAmp;
        TemplatePhasor templatePhasor;
    };
    // The block below must be kept in sync
    GenSettings genPreview;
    GenSettings genQuick;
    QRectF areaSim; // The rectangle of the image view area (simulation coordinates)
    QSize outResolution; // The output will be rendered to this resolution

    qreal aspectRatio() const {return (qreal)outResolution.width() / (qreal)outResolution.height();} // Width / height
    QImage imgPreview;
    QImage imgQuick;
    qreal testVal = 1;

public:
    ImageGen();
    void SetMainWindow(MainWindow * mainWindowIn) {mainWindow = mainWindowIn;}
    void GeneratePreviewImage();
    void GenerateQuickImage();
    int GenerateImage(QImage &imageOut, GenSettings &genSet);
    EmArrangement *GetActiveArrangement();
    int InitViewAreas();

    int GetEmitterList(QVector<EmitterF> &emitters);
    static void DebugEmitterLocs(const QVector<EmitterI>& emittersImg);
    static void DebugEmitterLocs(const QVector<EmitterF> &emittersF);
    static EmArrangement DefaultArrangement();
    void setTargetImgPoints(qint32 imgPoints, GenSettings &genSet);
    void EmitterCountIncrease();
    void EmitterCountDecrease();

    void setDistOffsetF(qreal in) {s.distOffsetF = in;}
    qreal getDistOffsetF() const {return s.distOffsetF;}

signals:
    void ImageChanged(QImage & image, qreal imgPerSimUnitOut);
    void EmittersChanged(); // Emitted when the emitter locations change

private:
    static void CalcDistArr(double simUnitPerIndex, Double2D_C &arr);
    static void CalcAmpArr(double distOffset, const Double2D_C &distArr, Double2D_C &ampArr);
    static void CalcPhasorArr(TemplatePhasor& templatePhasor,
                              const Double2D_C & templateDist, const Double2D_C & templateAmp);
    static QRgb ColourAngleToQrgb(int32_t angle, uint8_t alpha = 255);
    void AddPhasorArr(double imgPerSimUnit, double wavelength, EmitterI e, const Double2D_C & templateDist,
                      const Double2D_C & templateAmp, Complex2D_C & phasorArr);
    static void AddPhasorArr(EmitterI e, const Double2D_C &templateDist, const Double2D_C &templateAmp, const Complex2D_C &templatePhasor, Complex2D_C &phasorArr);
    static int EmitterArrangementToLocs(const EmArrangement &arngmt, QVector<QPointF> &emLocsOut);
    void CalcDistTemplate(QRect templateRect, GenSettings &genSet);
    void CalcAmpTemplate(qreal distOffset, GenSettings &genSet);
    void CalcPhasorTemplate(QRect templateRect, GenSettings &genSet);
public:

};

extern ImageGen imageGen;

#endif // IMAGEGEN_H
