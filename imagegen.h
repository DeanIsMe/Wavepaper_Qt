#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QPainter>
#include <QGraphicsView>
#include "datatypes.h"

void ImageDataDealloc(void * info);

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
    // (Doesn't affect the actual patten)
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
    static constexpr qint32 imgPointsQuick = 100000;
    static constexpr qint32 imgPointsPreview = 500000;
    static constexpr qreal templateOversizeFactor = 1.2; // The amount of extra length that the templates are calculated for (to prevent repeated recalculations)

private:
    MainWindow * mainWindow = nullptr;
    QList<EmArrangement> arngmtList;
    bool pendingQuickImage; // True if a quick image is pending to be generated
    bool pendingPreviewImage; // True if a preview image is pending to be generated
    bool hideEmitters = false; // When true, the emitters are not drawn on the preview window

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
    struct SumArray {
        Complex2D_C * phasorArr = nullptr; // Resultant phasor of all emitters summed together
        Double2D_C * ampArr = nullptr; // Amplitude of each point in sumArr
        double ampMax = 0;
        double ampMin = 999999;
        qint32 checkSum = 0;
    };

public:
    Settings s;

    struct GenSettings {
        double targetImgPoints = imgPointsPreview; // Total number of points in the preview. Change with setTargetImgPoints()
        double imgPerSimUnit; // The imgPerSimUnit that this template was generated with
        QRect areaImg; // The rectangle of the image view area (image coordinates)
        TemplateDist templateDist;
        TemplateAmp templateAmp;
        TemplatePhasor templatePhasor;
        SumArray combinedArr;
        bool indexedClr = true; // True for faster (but less accurate) colour map
    };

    // The block below must be kept in sync
    GenSettings genPreview;
    GenSettings genQuick;
    QRectF areaSim; // The rectangle of the image view area (simulation coordinates)
    QSize outResolution; // The output will be rendered to this resolution
    bool saveWithTransparency = false; // If true, when an image with a mask is saved, it will be saved with transparency. If false, then the background colour will be rendered into the image

    qreal aspectRatio() const {return s.view.aspectRatio;} // Width / height
    QImage imgPreview;
    QImage imgQuick;
    qint32 testVal = 1;

public:
    ImageGen();
    void SetMainWindow(MainWindow * mainWindowIn) {mainWindow = mainWindowIn;}
    void NewPreviewImageNeeded();
    void NewQuickImageNeeded();
    void NewImageNeeded();
    int GenerateImage(QImage &imageOut, GenSettings &genSet);
    EmArrangement *GetActiveArrangement();
    int InitViewAreas();

    int GetEmitterList(QVector<EmitterF> &emitters);
    static void DebugEmitterLocs(const QVector<EmitterI>& emittersImg);
    static void DebugEmitterLocs(const QVector<EmitterF> &emittersF);
    static EmArrangement DefaultArrangement();
    void setTargetImgPoints(qint32 imgPoints, GenSettings &genSet) const;

    void setDistOffsetF(qreal in) {s.distOffsetF = in;}
    qreal getDistOffsetF() const {return s.distOffsetF;}
    bool EmittersHidden();

    void SaveImage();

signals:
    void NewImageReady(QImage & image, qreal imgPerSimUnitOut, QColor backgroundClr); // A new image is ready
    void EmitterArngmtChanged(); // Emitted when the emitter locations change
    void GenerateImageSignal(); // Just used to queue up GenerateImageSlot
    void OverlayTextSignal(QString text);

public slots:
    void EmitterCountDecrease();
    void EmitterCountIncrease();
    void WavelengthDecrease();
    void WavelengthIncrease();
    void HideEmitters(bool hide) {
        hideEmitters = hide;
        emit EmitterArngmtChanged();
    }

private slots:
    void GenerateImageSlot();

private:
    static void CalcDistArr(double simUnitPerIndex, Double2D_C &arr);
    static void CalcAmpArr(double distOffset, const Double2D_C &distArr, Double2D_C &ampArr);
    static void CalcPhasorArr(TemplatePhasor& templatePhasor,
                              const Double2D_C & templateDist, const Double2D_C & templateAmp);
    static QRgb ColourAngleToQrgb(int32_t angle, uint8_t alpha = 255);
    void AddPhasorArr(double imgPerSimUnit, double wavelength, EmitterI e, const Double2D_C & templateDist,
                      const Double2D_C & templateAmp, Complex2D_C & phasorArr);
    static void AddPhasorArr(const EmitterI& e, const Double2D_C &templateDist, const Double2D_C &templateAmp, const Complex2D_C &templatePhasor, Complex2D_C &phasorArr);
    static int EmitterArrangementToLocs(const EmArrangement &arngmt, QVector<QPointF> &emLocsOut);
    void CalcDistTemplate(QRect templateRect, GenSettings &genSet);
    void CalcAmpTemplate(qreal distOffset, GenSettings &genSet);
    void CalcPhasorTemplate(QRect templateRect, GenSettings &genSet);
};

extern ImageGen imageGen;

#endif // IMAGEGEN_H
