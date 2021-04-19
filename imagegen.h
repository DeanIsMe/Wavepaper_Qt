#ifndef IMAGEGEN_H
#define IMAGEGEN_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QList>
#include <QPainter>
#include <QGraphicsView>
#include "datatypes.h"
#include "colourmap.h"

void ImageDataDealloc(void * info);


/** ****************************************************************************
 * @brief The ImageGen class
 */
class ImageGen : public QObject
{
    Q_OBJECT
public:
    // PROGRAM SETTINGS

    static constexpr qreal templateOversizeFactor = 1.2; // The amount of extra length that the templates are calculated for (to prevent repeated recalculations)
    static constexpr int trigTableLen = 10000;

private:
    MainWindow * mainWindow = nullptr;
    bool pendingQuickImage; // True if a quick image is pending to be generated
    bool pendingPreviewImage; // True if a preview image is pending to be generated
    bool hideEmitters = true; // When true, the emitters are not drawn on the preview window
    double sinTable[trigTableLen]; // Pre-calculated sin values. Indices 0 to trigTableLen correspond to 0 to 2pi rad
    double cosTable[trigTableLen]; // Pre-calculated cos values. Indices 0 to trigTableLen correspond to 0 to 2pi rad
    inline double sinQuick(double rad) {return sinTable[modPos((int)(rad*(double)trigTableLen/(2.*PI)),trigTableLen)];}
    inline double cosQuick(double rad) {return cosTable[modPos((int)(rad*(double)trigTableLen/(2.*PI)),trigTableLen)];}

public:
    Settings s; // Contains entire setup

    // The block below must be kept in sync
    GenSettings genPreview;
    GenSettings genQuick;
    QRectF areaSim; // The rectangle of the image view area (simulation coordinates)

    // For the final rendered image:
    QSize outResolution; // The output will be rendered to this resolution
    bool saveWithTransparency = false; // If true, when an image with a mask is saved, it will be saved with transparency. If false, then the background colour will be rendered into the image

    qreal aspectRatio() const {return s.view.aspectRatio;} // Width / height
    QImage imgPreview;
    QImage imgQuick;
    qint32 testVal = 1;
    ColourMap colourMap;

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

    void SaveImage(); // Saves to a file
    void AddArrangement(EmArrangement emArrangementIn); // Adds the given emitter arrangement

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
    bool GetHideEmitters() { return hideEmitters; }

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

    void PreCalcTrigTables();
    int GenerateImageWaves(QImage &imageOut, GenSettings &genSet);
    int GenerateImageFourBar(QImage &imageOut, GenSettings &genSet);
};

extern ImageGen imageGen;

#endif // IMAGEGEN_H
