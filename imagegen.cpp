#include "imagegen.h"
#include "colourmap.h"
#include "mainwindow.h"
#include <QList>
#include <QElapsedTimer>
#include <QImage>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include <previewscene.h>

ImageGen imageGen;

/** ****************************************************************************
 * @brief ImageGen::ImageGen
 */
ImageGen::ImageGen() : act(this) {
    QObject::connect(this, &ImageGen::GenerateImageSignal,
                     this, &ImageGen::GenerateImageSlot, Qt::QueuedConnection);
    InitViewAreas();
}

/** ****************************************************************************
 * @brief ImageGen::NewImageNeeded
 */
void ImageGen::NewImageNeeded() {
    // The quick image should be drawn first, and the preview image after
    pendingQuickImage = true;
    pendingPreviewImage = true;
    emit GenerateImageSignal(); // Process it later in the event queue
}

/** ****************************************************************************
 * @brief ImageGen::NewPreviewImageNeeded
 */
void ImageGen::NewPreviewImageNeeded() {
    pendingPreviewImage = true;
    emit GenerateImageSignal(); // Process it later in the event queue
}

/** ****************************************************************************
 * @brief ImageGen::NewQuickImageNeeded
 */
void ImageGen::NewQuickImageNeeded() {
    pendingQuickImage = true;
    emit GenerateImageSignal(); // Process it later in the event queue
}


/** ****************************************************************************
 * @brief ImageGen::GenerateImageSlot
 */
void ImageGen::GenerateImageSlot()
{
    // Draw a 'quick' image with a higher priority than the more detailed 'preview' image
    bool handled = false;
    if (pendingQuickImage) {
        handled = true;
        GenerateImage(imgQuick, genQuick);
        emit NewImageReady(imgQuick, genQuick.imgPerSimUnit);
        pendingQuickImage = false;
    }
    if (pendingPreviewImage) {
        if (handled) {
            emit GenerateImageSignal(); // Process it next time
        }
        else {
            handled = true;
            GenerateImage(imgPreview, genPreview);
            emit NewImageReady(imgPreview, genPreview.imgPerSimUnit);
            pendingPreviewImage = false;
        }
    }
}

/** ****************************************************************************
 * @brief ImageGen::InitViewAreas
 * @return
 */
int ImageGen::InitViewAreas() {
    outResolution = QSize(1080, 1920);
    areaSim = QRectF(0, 0, 100, 100. / aspectRatio());
    areaSim.moveCenter(QPoint(0,0));

    setTargetImgPoints(imgPointsPreview, genPreview);
    setTargetImgPoints(imgPointsQuick, genQuick);
    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::setTargetImgPoints
 * @param imgPoints
 */
void ImageGen::setTargetImgPoints(qint32 imgPoints, GenSettings & genSet) {
    // Determine the viewing window in image coordinates
    genSet.targetImgPoints = imgPoints;
    genSet.imgPerSimUnit = sqrt(genSet.targetImgPoints / areaSim.width() / areaSim.height());
    QRectF areaImgF(areaSim.topLeft() * genSet.imgPerSimUnit, areaSim.bottomRight() * genSet.imgPerSimUnit);
    genSet.areaImg = areaImgF.toRect();

    if (abs(areaSim.width() / areaSim.height() /
            (qreal)genSet.areaImg.width() * (qreal)genSet.areaImg.height() - 1) > 0.02) {
        qFatal("setTargetImgPoints: viewWindow and simWindow are different ratios!");
    }
}

/** ****************************************************************************
 * @brief ImageGen::EmitterCountIncrease
 */
void ImageGen::EmitterCountIncrease() {
    EmArrangement * group = GetActiveArrangement();
    group->count = std::max(group->count + 1, qRound((qreal)group->count * 1.2));
    NewImageNeeded();
    emit EmitterArngmtChanged();
}


/** ****************************************************************************
 * @brief ImageGen::EmitterCountDecrease
 */
void ImageGen::EmitterCountDecrease() {
    EmArrangement * group = GetActiveArrangement();
    int prevVal = group->count;
    group->count = std::max(1, std::min(group->count - 1, qRound((qreal)group->count * 0.8)));
    if (group->count != prevVal) {
        NewImageNeeded();
        emit EmitterArngmtChanged();
    }
}

/** ****************************************************************************
 * @brief ImageGen::WavelengthDecrease
 */
void ImageGen::WavelengthDecrease()
{
    this->s.wavelength *= 0.8;
    NewImageNeeded();
}

/** ****************************************************************************
 * @brief ImageGen::WavelengthIncrease
 */
void ImageGen::WavelengthIncrease()
{
    this->s.wavelength *= 1.25;
    NewImageNeeded();
}

/** ****************************************************************************
 * @brief ImageDataDealloc is a small utility function to clean up mem alloc
 * @note Function is of type QImageCleanupFunction
 * @param info
 */
void ImageDataDealloc(void * info) {
    if (info) {
        delete static_cast<Rgb2D_C*>(info);
    }
}

/** ****************************************************************************
 * @brief ImageGen::GenerateImage
 * @param imgRect - the range that the result will be generated for, in image coordinates
 * @param emitters
 * @param imageOut is the output
 * @return
 */
int ImageGen::GenerateImage(QImage& imageOut, GenSettings& genSet) {
    QElapsedTimer fnTimer;
    fnTimer.start();
    // This function works in image logical coordinates, which are integers

    QVector<EmitterF> emittersF;
    if (GetEmitterList(emittersF)) {
        return -2;
    }
    if (emittersF.size() == 0) {
        qWarning("fillImageData: No emitters! Abort drawing");
        return -1;
    }
    // Convert emLocs to image coordinates
    QVector<EmitterI> emittersImg(emittersF.size());
    for (int32_t i = 0; i < emittersF.size(); i++) {
        emittersImg[i] = EmitterI(emittersF[i], genSet.imgPerSimUnit);
    }

//    qDebug() << "Simulation window " << RectFToQString(areaSim) << "[sim units]";
//    qDebug() << "   Image size " << RectToQString(genSet.areaImg) << "[img units]";
//    qDebug("imgPerSimUnit = %.2f. numpoints", genSet.imgPerSimUnit);

    // TEMPLATES

    // Determine the range of the offset template
    QRect templateRect(0,0,0,0);
    for (EmitterI e : emittersImg) {
        templateRect |= genSet.areaImg.translated(-e.loc);
        // !@# need to upgrade the use of this template function to avoid crazy big arrays
    }

    // Distance template
    bool templatDistChanged = false;
    if (!genSet.templateDist.arr || !genSet.templateDist.arr->rect().contains(templateRect) ||
            genSet.imgPerSimUnit != genSet.templateDist.imgPerSimUnit) {
        // Must recalculate distance template
        CalcDistTemplate(templateRect, genSet);
        templatDistChanged = true;
    }

    // Amplitude template
    bool templatAmpChanged = false;
    qreal distOffset = (qreal)(genSet.areaImg.height() + genSet.areaImg.width()) / 2. * s.distOffsetF / genSet.imgPerSimUnit;
    if (templatDistChanged || !genSet.templateAmp.arr ||
            (genSet.templateAmp.arr->rect() != genSet.templateDist.arr->rect()) ||
            genSet.templateAmp.distOffset != distOffset) {
        // Must recalculate amplitude template
        CalcAmpTemplate(distOffset, genSet);
        templatAmpChanged = true;
    }

    // Phasor template
    if (templatAmpChanged || !genSet.templatePhasor.arr ||
            genSet.imgPerSimUnit != genSet.templatePhasor.imgPerSimUnit ||
            s.wavelength != genSet.templatePhasor.wavelength ||
            !genSet.templatePhasor.arr->rect().contains(templateRect)) {
        CalcPhasorTemplate(templateRect, genSet);
    }

    auto timePostTemplates = fnTimer.elapsed();

    // CALC PHASOR SUM

    // Generate a map of the phasors for each emitter, and sum together
    // Use the distance and amplitude templates
    Complex2D_C * phasorSumArr = new Complex2D_C(genSet.areaImg);
    for (EmitterI e : emittersImg) {
        AddPhasorArr(e, *genSet.templateDist.arr, *genSet.templateAmp.arr, *genSet.templatePhasor.arr, *phasorSumArr);
    }

    auto timePostPhasors = fnTimer.elapsed();
    qint64 timePostColorIndex, timePostPhasorMag;

    // COLOUR MAP

    // Scaler will be 1/[emitter amplitude at half the simulation width]
    double scaler = 1/genSet.templateAmp.arr->getPoint(genSet.areaImg.width() / 2, 0);

    // Use the resultant amplitude to fill in the pixel data
    Rgb2D_C* pixArr = new Rgb2D_C(genSet.areaImg);
    if (0) {
        // Colour angle (basic)
        for (int y = pixArr->yTop; y < pixArr->yTop + pixArr->height; y++) {
            for (int x = pixArr->xLeft; x < pixArr->xLeft + pixArr->width; x++) {
                pixArr->setPoint(x, y, ColourAngleToQrgb(std::abs(phasorSumArr->getPoint(x, y)) * scaler * 1530 * 0.2, 255));
            }
        }
    }
    else {
        // Colour map
        timePostColorIndex = fnTimer.elapsed();

        // Find max and min values
        Double2D_C amplitude(phasorSumArr->rect());
        qreal maxAmp = 0;
        qreal minAmp = genSet.templateAmp.arr->getPoint(1,1);

        for (int y = pixArr->yTop; y < pixArr->yTop + pixArr->height; y++) {
            for (int x = pixArr->xLeft; x < pixArr->xLeft + pixArr->width; x++) {
                qreal amp = std::abs(phasorSumArr->getPoint(x, y));
                amplitude.setPoint(x, y, amp);
                minAmp = std::min(minAmp, amp);
                maxAmp = std::max(maxAmp, amp);
            }
        }

        timePostPhasorMag = fnTimer.elapsed();

        qreal mult = 1. / (maxAmp - minAmp);
        for (int y = pixArr->yTop; y < pixArr->yTop + pixArr->height; y++) {
            for (int x = pixArr->xLeft; x < pixArr->xLeft + pixArr->width; x++) {
                // Calculate location in range 0 to 1;
                qreal loc = (amplitude.getPoint(x, y) - minAmp) * mult;
                pixArr->setPoint(x, y, colourMap.GetColourValue(loc));
            }
        }
    }

    auto timePostClrMap = fnTimer.elapsed();

    if (0) {
        // Test pattern, depends only on location
        for (int y = pixArr->yTop; y < pixArr->yTop + pixArr->height; y++) {
            for (int x = pixArr->xLeft; x < pixArr->xLeft + pixArr->width; x++) {
                pixArr->setPoint(x, y,
                                 ColourAngleToQrgb(((x - pixArr->xLeft) * 1530) / pixArr->width +
                                                   ((y - pixArr->yTop) * 1000) / pixArr->height, 255));
            }
        }
    }
    imageOut = QImage((uchar*)pixArr->getDataPtr(), pixArr->width, pixArr->height, QImage::Format_ARGB32,
                      &ImageDataDealloc, pixArr);
                      // QRgb is ARGB32 (8 bits per channel)

    auto timePostImage = fnTimer.elapsed();

    delete phasorSumArr;
    qDebug("ImageGen::GenerateImage took %4lld ms. Setup=%4lldms, PhasorMap=%4lldms, Colouring=%4lldms, Image=%4lldms",
           fnTimer.elapsed(), timePostTemplates, timePostPhasors - timePostTemplates,
           timePostClrMap - timePostPhasors,
           timePostImage - timePostClrMap);



    QString clrTime = QString::asprintf("Colouring=%4lldms. %4lldms index, %4lldms mag, %4lldms clrMap",
                      timePostClrMap - timePostPhasors,
                      timePostColorIndex - timePostPhasors,
                      timePostPhasorMag - timePostColorIndex,
                      timePostClrMap - timePostPhasorMag);
    qDebug() << clrTime; // !@#$
    mainWindow->textWindow->appendPlainText(clrTime);

    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::CalcDistTemplate
 * @param templateRect is the minimum required size
 * @param genSet is image generation settings
 */
void ImageGen::CalcDistTemplate(QRect templateRect, GenSettings & genSet) {
    // Make the template size 20% bigger (to prevent very frequent calculation)
    QPoint center = templateRect.center();
    templateRect.setSize(templateRect.size() * templateOversizeFactor);
    templateRect.moveCenter(center);
    qDebug() << "Recalculating dist template for range " << RectToQString(templateRect);

    // Generate a template array of distance (scene units)
    if (genSet.templateDist.arr) {delete genSet.templateDist.arr;}
    genSet.templateDist.arr = new Double2D_C(templateRect);
    CalcDistArr(1. / genSet.imgPerSimUnit, *genSet.templateDist.arr);
    genSet.templateDist.imgPerSimUnit = genSet.imgPerSimUnit;
}

/** ****************************************************************************
 * @brief ImageGen::CalcAmpTemplate
 * @param distOffset is the value 'a' in the amplitude equation: 1/(r+a)
 * @param genSet is image generation settings
 */
void ImageGen::CalcAmpTemplate(qreal distOffset, GenSettings & genSet) {
    // The size of the amplitude template is tied to the distance template
    QRect templateRect = genSet.templateDist.arr->rect();
    qDebug() << "Recalculating dist template for range " << RectToQString(templateRect);

    // Generate a template array of the amplitudes
    if (genSet.templateAmp.arr) {delete genSet.templateAmp.arr;}
    genSet.templateAmp.arr = new Double2D_C(templateRect);
    CalcAmpArr(distOffset, *genSet.templateDist.arr, *genSet.templateAmp.arr);
    genSet.templateAmp.distOffset = distOffset;
}

/** ****************************************************************************
 * @brief ImageGen::CalcPhasorTemplate
 * @param templateRect
 */
void ImageGen::CalcPhasorTemplate(QRect templateRect, GenSettings & genSet) {
    // Make the template size 20% bigger (to prevent very frequent calculation)
    QPoint center = templateRect.center();
    templateRect.setSize(templateRect.size() * templateOversizeFactor);
    templateRect.moveCenter(center);
    // Prevent the phasor template from being larger than the distance and amplitude
    if (!genSet.templateDist.arr->rect().contains(templateRect)) {
        templateRect = genSet.templateDist.arr->rect();
    }

    qDebug() << "Recalculating phasor template for range " << RectToQString(templateRect);
    // Template array of phasors
    genSet.templatePhasor.MakeNew(templateRect, s.wavelength, genSet.imgPerSimUnit);
    CalcPhasorArr(genSet.templatePhasor, *genSet.templateDist.arr, *genSet.templateAmp.arr);
}

/** ****************************************************************************
 * @brief ImageGen::GetActiveArrangement
 * @return
 */
EmArrangement* ImageGen::GetActiveArrangement()
{
    if (arngmtList.size() == 0) {
        return nullptr;
    }
    return &arngmtList.last();
}

/** ****************************************************************************
 * @brief ImageGen::CalcDistArr
 * @param simUnitPerIndex is the distance gap between each index
 * @param arr
 */
void ImageGen::CalcDistArr(double simUnitPerIndex, Double2D_C & arr) {
    // Calculate the distance at every point

    for (int32_t y = arr.yTop; y < arr.yTop + arr.height; y++) {
        for (int32_t x = arr.xLeft; x < arr.xLeft + arr.width; x++) {
            arr.setPoint(x, y, simUnitPerIndex * sqrt((x*x) + (y*y)));
        }
    }
    return;
}


/** ****************************************************************************
 * @brief ImageGen::CalcAmpArr
 * @param distOffset is the offset added to each distance. It controls linearity
 * @param distArr is the distance template array
 * @param ampArr is the output amplitude template array
 */
void ImageGen::CalcAmpArr(double distOffset, const Double2D_C & distArr, Double2D_C & ampArr) {
    // Calculate the complex phasor at every point
    if (distArr.rect() != ampArr.rect()) {
        qFatal("calcAmpArr distArr != ampArr! (not supported, but it could be)");
        return;
    }
    if (distOffset == INFINITY) {
        // Constant amplitude
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1.);
            }
        }
    }
    else {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1/(distArr.getPoint(x, y) + distOffset));
            }
        }
    }
}

/** ****************************************************************************
 * @brief ImageGen::CalcPhasorArr calculates a phasor template over the same
 * range as the distance and amplitude templates
 * @param templatePhasor
 * @param templateDist
 * @param templateAmp
 */
void ImageGen::CalcPhasorArr(TemplatePhasor& templatePhasor,
                             const Double2D_C & templateDist, const Double2D_C & templateAmp) {
    Complex2D_C& arr = *templatePhasor.arr; // Output phasor array
    // Checks
    if (!templateDist.rect().contains(arr.rect())) {
        qFatal("CalcPhasorArr templateDist != templatePhasor! (not supported, but it could be)");
        return;
    }
    if (!templateAmp.rect().contains(arr.rect())) {
        qFatal("CalcPhasorArr templateAmp != templatePhasor! (not supported, but it could be)");
        return;
    }

    // templateDist is in image units (pixels)
    double radPerSim = -2 * PI / templatePhasor.wavelength; // Radians per unit distance, * -1

    for (int32_t y = arr.yTop; y < arr.yTop + arr.height; y++) {
        for (int32_t x = arr.xLeft; x < arr.xLeft + arr.width; x++) {
            arr.setPoint(x, y, std::polar<fpComplex>(templateAmp.getPoint(x,y),
                                                          (templateDist.getPoint(x, y)) * radPerSim));
            //            fpComplex amp = templateAmp.getPoint(x,y);
            //            qreal angle = (templateDist.getPoint(x, y) + distOffsetImg) * radPerImg;
            //            phasorArr.addPoint(x, y, complex(amp * cos(angle), amp * sin(angle)));
        }
    }
    return;
}

/** ****************************************************************************
 * @brief ImageGen::AddPhasorArr for 1 emitter, calculates the phasor at every
 * location in the view window and add it to phasorArr
 * @param wavelength (sim units)
 * @param distOffset causes a constant phase shift on every point
 * @param templateDist contains pre-calculated distance values for a given offset
 * @param templateAmp contains pre-calculated amplitude values for a given offset
 * @param emLoc is the emitter location that this phasor array is calculated for
 * @param phasorArr is the output array. It also defines the view window
 */
void ImageGen::AddPhasorArr(double imgPerSimUnit, double wavelength, EmitterI e,
                             const Double2D_C & templateDist, const Double2D_C & templateAmp,
                            Complex2D_C & phasorArr) {
    // phasorArray dimensions are that of the image. emLoc is the location we're calculating for
    // This function is performed in a shifted coordinate system (for efficiency)
    // The new coordinate system is 1:1 scale, but has the origin shifted such that the emitter is @ (0,0)
    QRect rect = phasorArr.rect().translated(-e.loc); // image rect with emitter location is the center
    // Distort the phasorArr coordinates during this function, such that the emitter is at the center
    phasorArr.translate(-e.loc);

    // Checks
    if (!templateDist.rect().contains(phasorArr.rect())) {
        qFatal("addPhasorArr - templateDist doesn't contain required offsets!");
        return;
    }
    if (!templateAmp.rect().contains(phasorArr.rect())) {
        qFatal("addPhasorArr - templateAmp doesn't contain required offsets!");
        return;
    }

    // templateDist is in image units (pixels)
    qreal distOffsetImg = e.distOffset * imgPerSimUnit;
    double radPerImg = -2 * PI / (wavelength * imgPerSimUnit); // Radians per unit distance, * -1
    for (int32_t y = rect.top(); y < rect.top() + rect.height(); y++) {
        for (int32_t x = rect.x(); x < rect.x() + rect.width(); x++) {
            phasorArr.addPoint(x, y,std::polar<fpComplex>(templateAmp.getPoint(x,y),
                                                          (templateDist.getPoint(x, y) + distOffsetImg) * radPerImg));
//            fpComplex amp = templateAmp.getPoint(x,y);
//            qreal angle = (templateDist.getPoint(x, y) + distOffsetImg) * radPerImg;
//            phasorArr.addPoint(x, y, complex(amp * cos(angle), amp * sin(angle)));
        }
    }
    // Restore the phasorArr coordinates
    phasorArr.translate(e.loc);
    return;
}

void ImageGen::AddPhasorArr(EmitterI e, const Double2D_C & templateDist,
                            const Double2D_C & templateAmp, const Complex2D_C & templatePhasor,
                            Complex2D_C & phasorArr) {
    // phasorArray dimensions are that of the image. emLoc is the location we're calculating for
    // This function is performed in a shifted coordinate system (for efficiency)
    // The new coordinate system is 1:1 scale, but has the origin shifted such that the emitter is @ (0,0)
    QRect rect = phasorArr.rect().translated(-e.loc); // image rect with emitter location is the center
    // Distort the phasorArr coordinates during this function, such that the emitter is at the center
    phasorArr.translate(-e.loc);

    // Checks
    if (!templateDist.rect().contains(phasorArr.rect())) {
        qFatal("addPhasorArr - templateDist doesn't contain required offsets!");
        return;
    }
    if (!templateAmp.rect().contains(phasorArr.rect())) {
        qFatal("addPhasorArr - templateAmp doesn't contain required offsets!");
        return;
    }
    if (!templatePhasor.rect().contains(phasorArr.rect())) {
        qFatal("addPhasorArr - templateAmp doesn't contain required offsets!");
        return;
    }

    // templateDist is in image units (pixels)
    for (int32_t y = rect.top(); y < rect.top() + rect.height(); y++) {
        for (int32_t x = rect.x(); x < rect.x() + rect.width(); x++) {
            phasorArr.addPoint(x, y, templatePhasor.getPoint(x, y));
//            fpComplex amp = templateAmp.getPoint(x,y);
//            qreal angle = (templateDist.getPoint(x, y) + distOffsetImg) * radPerImg;
//            phasorArr.addPoint(x, y, complex(amp * cos(angle), amp * sin(angle)));
        }
    }
    // Restore the phasorArr coordinates
    phasorArr.translate(e.loc);
    return;
}


/** ****************************************************************************
 * @brief ImageGen::ColourAngleToQrgb sets a red, green and blue bytes to a point on a colour wheel
 *        This is useful for a simple colour map
 * @param angle - ranges from 0 to 1529, representing the entire colour wheel
 * @param alpha is the alpha value - just passed on to the colour
 * @return
 */
QRgb ImageGen::ColourAngleToQrgb(int32_t angle, uint8_t alpha) {
    angle = angle % 1530; // 1530 steps/values total.
    int32_t stage = angle/255; // 0 to 5
    uint8_t colour[3];
    // Set the starting levels for this stage
    // for (u8 i=0; i<3; i++) {*colour[i] = ((stage+2*(i+1))%6)/3 ? 255 : 0;} // Loop version
    colour[0] = ((stage+2)%6)/3 ? 255 : 0;
    colour[1] = ((stage+4)%6)/3 ? 255 : 0;
    colour[2] = ((stage+6)%6)/3 ? 255 : 0;
    colour[stage%3] += stage%2 ? -(angle%255) : angle%255;

    return qRgba(colour[0], colour[1], colour[2], alpha);
}


/** ****************************************************************************
 * @brief EmitterArrangementToLocs determines and returns the locations of the emitters, given the arrangement
 * @param arngmt
 * @param emLocsOut
 * @return
 */
int ImageGen::EmitterArrangementToLocs(const EmArrangement & arngmt, QVector<QPointF> & emLocsOut) {
    switch (arngmt.type) {
    case EmType::blank:
        emLocsOut.resize(0);
        break;
    case EmType::arc: {
        // Think of each emitter as occupying an angle 'emGap', with the emitter
        // at the center. The total angle span occupied by all will be arcSpan.
        // This means the outer 2 emitters are NOT at teh 2 edges of arcSpan.
        // E.g. 3 emitters across a span of 1.2. will result in locations of -0.4, 0.0, and +0.4.
        // (not -0.6, 0.0, and +0.6).
        emLocsOut.resize(arngmt.count);
        const qreal emGap = arngmt.arcSpan / (qreal)arngmt.count;
        const qreal startAng = 3 * 3.1415926/2 + emGap * 0.5 * (1 - arngmt.count);
        for (int32_t i = 0; i < emLocsOut.size(); i++) {
            double angle = startAng + emGap * i;
            emLocsOut[i] = QPointF(arngmt.arcRadius * cos(angle), arngmt.arcRadius * sin(angle));
        }
        break;
    }
    case EmType::line:
        emLocsOut.resize(arngmt.count);
        if (arngmt.count == 1) {
            emLocsOut[0] = QPointF(0, 0);
        }
        else {
            double step = 1.0/(arngmt.count-1.0);
            for (int32_t i = 0; i < emLocsOut.size(); i++) {
                emLocsOut[i] = QPointF(arngmt.lenTotal * (-0.5 + i * step), 0);
            }
        }
        break;
    case EmType::square:
        // !@#$ implement!
        break;
    case EmType::custom:
        emLocsOut = arngmt.customLocs;
        break;
    }

    // Rotate and translate the scatterers, if needed (about the origin)
    QTransform transform;
    transform.rotate(-arngmt.rotation);
    transform.translate(arngmt.center.x(), arngmt.center.y());
    for (QPointF &p : emLocsOut) {
        p = transform.map(p)   ;
    }

    // Mirror
    if (arngmt.mirrorHor) {
        int32_t len = emLocsOut.size();
        emLocsOut.resize(len * 2);
//        QTransform mirror(-1., 0., 0., 1., 0, 0);
//        for (int32_t i = 0; i < len; i++) {
//            emLocsOut[len + i] = mirror.map(emLocsOut[i]);
//        }
        for (int32_t i = 0; i < len; i++) {
            emLocsOut[len + i] = QPointF(-emLocsOut[i].x(), emLocsOut[i].y());
        }
    }
    if (arngmt.mirrorVert) {
        int32_t len = emLocsOut.size();
        emLocsOut.resize(len * 2);
//        QTransform mirror(1., 0., 0., -1., 0, 0);
//        for (int32_t i = 0; i < len; i++) {
//            emLocsOut[len + i] = mirror.map(emLocsOut[i]);
//        }

        for (int32_t i = 0; i < len; i++) {
            emLocsOut[len + i] = QPointF(emLocsOut[i].x(), -emLocsOut[i].y());
        }
    }
    return 0;
}


/** ****************************************************************************
 * @brief GetEmitterList creates a vector holding the locations of all emitters
 * generated from the arrangements
 * @param emittersImg
 */
int ImageGen::GetEmitterList(QVector<EmitterF> & emitters) {

    if (arngmtList.size() == 0) {
        arngmtList.append(DefaultArrangement());
    }

    // Build a vector of all emitter locations from the arrangements
    QVector<QPointF> emLocs;
    for (EmArrangement arn : arngmtList) {
        QVector<QPointF> thisEmLocs;
        EmitterArrangementToLocs(arn, thisEmLocs);
        emLocs.append(thisEmLocs);
    }

    // Create emitters from the locations
    emitters.resize(emLocs.size());
    for (int32_t i = 0; i < emLocs.size(); i++) {
        emitters[i].loc = emLocs[i];
    }

    if (emitters.size() == 0) {
        qWarning("No emitters! Abort drawing");
        return -2;
    }

    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::DebugEmitterLocs
 * @param emittersImg
 */
void ImageGen::DebugEmitterLocs(const QVector<EmitterI> &emittersImg) {
    qDebug() << "Emitter locations (img):";
    QStringList strList;
    for (EmitterI e : emittersImg) {
        strList.append(e.ToString());
    }
    qDebug(strList.join('\n').toLatin1());
    qDebug("\n");
}

void ImageGen::DebugEmitterLocs(const QVector<EmitterF> &emittersF) {
    qDebug() << "Emitter locations (sim):";
    QStringList strList;
    for (EmitterF e : emittersF) {
        strList.append(e.ToString());
    }
    qDebug(strList.join('\n').toLatin1());
    qDebug("\n");
}

/** ****************************************************************************
 * @brief ImageGen::DefaultArrangement
 * @return
 */
EmArrangement ImageGen::DefaultArrangement() {
    EmArrangement arn;
    arn.type = EmType::arc;
    arn.arcRadius = 30;
    arn.arcSpan = 2.*PI/3.;
    arn.count = 5;
    arn.mirrorHor = arn.mirrorVert = false;
    arn.center = QPointF(0, 0);
    arn.count = 5;
    return arn;
}

/** ****************************************************************************
 * @brief ImageGen::Interact::mousePressEvent
 * @param event
 */
void ImageGen::Interact::mousePressEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene)
{
    Q_UNUSED(scene);
    qDebug("Scene press   event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    Cancel();
    grpActive = parent->GetActiveArrangement();
    if (!grpActive) {
        // No active groups
        return;
    }
    active = true;
    ctrlPressed = (event->modifiers() & Qt::ControlModifier);
    grpBackup = *grpActive; // Save, so that it can be reverted
    pressPos = event->scenePos();
}

/** ****************************************************************************
 * @brief ImageGen::Interact::mouseReleaseEvent
 * @param event
 * @param scene
 */
void ImageGen::Interact::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene)
{
    Q_UNUSED(scene);
    qDebug("Scene release event (%7.2f, %7.2f)", event-> scenePos().x(), event->scenePos().y());
    active = false;

    emit parent->EmitterArngmtChanged(); // Just need to redraw the axes lines when mirroring

    parent->NewPreviewImageNeeded();
}

/** ****************************************************************************
 * @brief ImageGen::Interact::mouseMoveEvent
 * @param event
 * @param scene
 */
void ImageGen::Interact::mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene)
{
    Q_UNUSED(scene);
    qDebug("Scene move    event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    if (!active) {return;}
    // Determine how much the mouse has moved while clicked
    QPointF delta = event->scenePos() - pressPos;

    // Alt disables snapping
    bool snapEn = (event->modifiers() & Qt::AltModifier) == 0;

    if (ctrlPressed) {
        // Change location
        grpActive->center = grpBackup.center + delta;
        if (snapEn) {
            grpActive->center.setX(Snap(grpActive->center.x(), 9e9, parent->areaSim.width() * 0.05));
            grpActive->center.setY(Snap(grpActive->center.y(), 9e9, parent->areaSim.width() * 0.05));
        }
    }
    else {
        switch (grpActive->type) {
        case EmType::arc: {
            grpActive->arcRadius = std::max(0.0, grpBackup.arcRadius - delta.y());
            qreal spanDelta = delta.x() / parent->areaSim.width() * 3.1415926 * 2;

            grpActive->arcSpan = std::max(0.0, grpBackup.arcSpan + spanDelta);
            if (snapEn) {
                grpActive->arcSpan = Snap(grpActive->arcSpan, PI, PI * 0.1);
            }
            break;
        }
        case EmType::line:
            grpActive->lenTotal = std::max(0.0, grpBackup.lenTotal - delta.y());
            break;
        default:
            return;
        }
    }

    emit parent->EmitterArngmtChanged();
    parent->NewQuickImageNeeded();
}

/** ****************************************************************************
 * @brief ImageGen::Interact::Cancel
 */
void ImageGen::Interact::Cancel() {
    // Restore the backup
    if (active) {
        if (grpActive) {
            *grpActive = grpBackup;
        }
    }
    active = false;
}

/** ****************************************************************************
 * @brief ImageGen::TemplatePhasor::MakeNew
 * @param size
 * @param wavelengthIn
 * @param imgPerSimUnitIn
 */
void ImageGen::TemplatePhasor::MakeNew(QRect size, qreal wavelengthIn, qreal imgPerSimUnitIn) {
    if (this->arr) { delete this->arr; }
    this->arr = new Complex2D_C(size);
    this->wavelength = wavelengthIn;
    this->imgPerSimUnit = imgPerSimUnitIn;
}
