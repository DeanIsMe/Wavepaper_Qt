#include "imagegen.h"
#include <QList>
#include <QElapsedTimer>
#include <QImage>
#include <QWidget>

ImageGen imageGen;

/** ****************************************************************************
 * @brief ImageGen::ImageGen
 */
ImageGen::ImageGen()
{
    InitViewAreas();
}

void ImageGen::GeneratePreview()
{
    imageGen.GenerateImage(imageGen.imgPreview);
    emit imageGen.PreviewImageChanged();
}

/**
 * @brief ImageGen::InitViewAreas
 * @return
 */
int ImageGen::InitViewAreas() {
    outResolution = QSize(1080, 1920);
    simArea = QRectF(0, 0, 100, 100. / aspectRatio());
    simArea.moveCenter(QPoint(0,0));

    // Determine the viewing window in image coordinates
    targetImgPoints = 200000;
    imgPerSimUnit = sqrt(targetImgPoints / simArea.width() / simArea.height());
    QRectF imgAreaF(simArea.topLeft() * imgPerSimUnit, simArea.bottomRight() * imgPerSimUnit);
    imgArea = imgAreaF.toRect();

    if (abs(simArea.width() / simArea.height() / (qreal)imgArea.width() * (qreal)imgArea.height() - 1) > 0.02) {
        qFatal("InitViewAreas: viewWindow and simWindow are different ratios!");
        return -1;
    }
    return 0;
}

/**
 * @brief ImageGen::setTargetImgPoints
 * @param imgPoints
 */
void ImageGen::setTargetImgPoints(qint32 imgPoints) {
    // Determine the viewing window in image coordinates
    targetImgPoints = imgPoints;
    imgPerSimUnit = sqrt(targetImgPoints / simArea.width() / simArea.height());
    QRectF imgAreaF(simArea.topLeft() * imgPerSimUnit, simArea.bottomRight() * imgPerSimUnit);
    imgArea = imgAreaF.toRect();

    if (abs(simArea.width() / simArea.height() / (qreal)imgArea.width() * (qreal)imgArea.height() - 1) > 0.02) {
        qFatal("setTargetImgPoints: viewWindow and simWindow are different ratios!");
    }
}

/**
 * @brief ImageGen::EmitterCountIncrease
 */
void ImageGen::EmitterCountIncrease() {
    EmArrangement * group = GetActiveArrangement();
    group->count = std::max(group->count + 1, qRound((qreal)group->count * 1.2));
    GenerateImage(imgPreview);
    emit PreviewImageChanged();
    emit EmittersChanged();
}

/**
 * @brief ImageGen::EmitterCountDecrease
 */
void ImageGen::EmitterCountDecrease() {
    EmArrangement * group = GetActiveArrangement();
    int prevVal = group->count;
    group->count = std::max(1, std::min(group->count - 1, qRound((qreal)group->count * 0.8)));
    if (group->count != prevVal) {
        GenerateImage(imgPreview);
        emit PreviewImageChanged();
        emit EmittersChanged();
    }
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
int ImageGen::GenerateImage(QImage& imageOut) {
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
        emittersImg[i] = EmitterI(emittersF[i], imgPerSimUnit);
    }

    qDebug() << "Simulation window " << RectFToQString(simArea) << "[sim units]";
    qDebug() << "   Image size " << RectToQString(imgArea) << "[img units]";
    qDebug("imgPerSimUnit = %.2f. numpoints", imgPerSimUnit);

    // Determine the range of the offset template
    QRect templateRect(0,0,0,0);
    for (EmitterI e : emittersImg) {
        templateRect |= imgArea.translated(-e.loc);
        // !@# need to upgrade the use of this template function to avoid crazy big arrays
    }

    if (!templateDist || !templateDist->getRect().contains(templateRect)) {
        // Must recalculate templates!
        // Make the template size 20% bigger (to prevent very frequent calculation)
        QPoint center = templateRect.center();
        templateRect.setSize(templateRect.size() * 1.2);
        templateRect.moveCenter(center);
        CalcTemplates(templateRect);
    }

    auto timePostTemplates = fnTimer.elapsed();

    // Generate a map of the phasors for each emitter, and sum together
    // Use the distance and amplitude templates
    Complex2D_C * phasorSumArr = new Complex2D_C(imgArea);

    for (EmitterI e : emittersImg) {
        AddPhasorArr(imgPerSimUnit, s.wavelength, e, *templateDist, *templateAmp, *phasorSumArr);
    }

    auto timePostPhasors = fnTimer.elapsed();

    // Scaler will be 1/[emitter amplitude at half the simulation width]
    double scaler = 1/templateAmp->getPoint(imgArea.width() / 2, 0);

    // Use the resultant amplitude to fill in the pixel data
    Rgb2D_C* pixArr = new Rgb2D_C(imgArea);
    for (int y = pixArr->yTop; y < pixArr->yTop + pixArr->height; y++) {
        for (int x = pixArr->xLeft; x < pixArr->xLeft + pixArr->width; x++) {
            pixArr->setPoint(x, y, ColourAngleToQrgb(std::abs(phasorSumArr->getPoint(x, y)) * scaler * 1530 * 0.2, 255));
        }
    }

    auto timePostClrMap = fnTimer.elapsed();

    if (0) {
        // Test gradient pattern
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
           timePostClrMap - timePostPhasors, timePostImage - timePostClrMap);
    return 0;
}

/** ****************************************************************************
 * @brief ImageGen::CalcTemplates
 * @param templateRect
 */
void ImageGen::CalcTemplates(QRect templateRect) {
    qDebug() << "Recalculating templates";
    qDebug() << "Template range is " << RectToQString(templateRect);
    // Generate a template array of distance (image units)
    if (templateDist) {delete templateDist;}
    templateDist = new Double2D_C(templateRect);
    CalcDistArr(1.0, *templateDist);

    // Generate a template array of the amplitudes, depending on the attenuation factor
    // templateAmp also uses image units (which means the value will be off by a factor of imgPerSimUnit^attnFactor
    if (templateAmp) {delete templateAmp;}
    templateAmp = new Double2D_C(templateRect);
    CalcAmpArr(s.attnFactor, *templateDist, *templateAmp);
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
 * @param attnFactor is normally 1. Amplitude drops off at rate of 1/(r^attnFactor). 1/r is standard.
 * @param arr
 */
void ImageGen::CalcAmpArr(double attnFactor, const Double2D_C & distArr, Double2D_C & ampArr) {
    // Calculate the complex phasor at every point
    if (distArr.getRect() != ampArr.getRect()) {
        qFatal("calcAmpArr distArr != ampArr! (not supported, but it could be)");
        return;
    }
    if (attnFactor == 0) {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1);
            }
        }
    }
    else if (attnFactor == 1) {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, 1/distArr.getPoint(x, y));
            }
        }
    }
    else {
        for (int32_t y = ampArr.yTop; y < ampArr.yTop + ampArr.height; y++) {
            for (int32_t x = ampArr.xLeft; x < ampArr.xLeft + ampArr.width; x++) {
                ampArr.setPoint(x, y, pow(distArr.getPoint(x, y), -attnFactor)); // 1/r^attnFactor = r^-attnFactor
            }
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
    QRect rect = phasorArr.getRect().translated(-e.loc); // image rect with emitter location is the center
    // Distort the phasorArr coordinates during this function, such that the emitter is at the center
    phasorArr.translate(-e.loc);

    // Checks
    if (!templateDist.getRect().contains(phasorArr.getRect())) {
        qFatal("addPhasorArr - templateDist doesn't contain required offsets!");
        return;
    }
    if (!templateAmp.getRect().contains(phasorArr.getRect())) {
        qFatal("addPhasorArr - templateAmp doesn't contain required offsets!");
        return;
    }

    // templateDist is in image units (pixels)
    qreal distOffsetImg = e.distOffset * imgPerSimUnit;
    double radPerImg = -2 * 3.14159265359 / (wavelength * imgPerSimUnit); // Radians per unit distance, * -1
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

/**
 * @brief ImageGen::DefaultArrangement
 * @return
 */
EmArrangement ImageGen::DefaultArrangement() {
    EmArrangement arn;
    arn.type = EmType::arc;
    arn.arcRadius = 30;
    arn.arcSpan = 3.14159/2;
    arn.count = 5;
    arn.mirrorHor = arn.mirrorVert = false;
    arn.center = QPointF(0, 0);
    arn.count = 5;
    return arn;
}
