#include "datatypes.h"
#include "interact.h"
#include "imagegen.h"
#include "colourmap.h"
#include "mainwindow.h"
#include "previewscene.h"
#include <QGraphicsSceneMouseEvent>

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/


/** ****************************************************************************
 * @brief Interact::Interact
 * @param imgGenIn
 */
Interact::Interact(MainWindow &mainWindowIn, ImageGen& imgGenIn) :
    mainWindow(mainWindowIn), imgGen(imgGenIn), colourMap(imgGenIn.colourMap) {
    SelectDefaultType();
}

/** ****************************************************************************
 * @brief Interact::SelectType
 * @param type
 */
void Interact::SelectType(Type type) {
    if (typeSelected != type) {
        typeSelected = type;
        emit InteractTypeChanged(QVariant(typeSelected));
        qDebug("Interact type changed! = %d", typeSelected);
    }
}

void Interact::DeselectType(Interact::Type type) {
    if (typeSelected == type) {
        SelectDefaultType();
    }
}


void Interact::SelectDefaultType()
{
    if (mainWindow.programMode == ProgramMode::waves) {
        SelectType(defaultTypeWaves); // Revert to default
    }
    else if (mainWindow.programMode == ProgramMode::fourBar) {
        SelectType(defaultTypeFourBar); // Revert to default
    }
}


/** ****************************************************************************
 * @brief Interact::mousePressEvent
 * @param event
 */
void Interact::mousePressEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene)
{
    Q_UNUSED(scene);
    qDebug("Mouse press   event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    Cancel();

    ctrlPressed = (event->modifiers() & Qt::ControlModifier);
    pressPos = event->scenePos();

    switch (typeSelected) {
        case Type::mask:
        // Interact with the mask
        maskConfigBackup = imgGen.s.maskCfg;
        break;
    case Type::colours:
        // No backup required
        clrListBackup = imgGen.s.clrList;
        break;
    case Type::arrangement:
        // Interact with the emitter arrangement
        grpActive = imgGen.GetActiveArrangement();
        if (!grpActive) {
            // No active groups
            return;
        }
        grpBackup = *grpActive; // Save, so that it can be reverted
        break;

    case Type::lengths:
    case Type::angleInc:
    case Type::position:
    case Type::drawRange:
    case Type::angleInit:
        fourBarBackup = imgGen.s.fourBar;
        break;

    case Type::null:
        return;
        break;
    }
    active = typeSelected;
}

/** ****************************************************************************
 * @brief Interact::mouseReleaseEvent
 * @param event
 * @param scene
 */
void Interact::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene)
{
    Q_UNUSED(scene);
    qDebug("Mouse release event (%7.2f, %7.2f)", event-> scenePos().x(), event->scenePos().y());

    switch (active) {
    case Type::null:
        break;

    case Type::arrangement:
        emit imgGen.EmitterArngmtChanged(); // Just need to redraw the axes lines when mirroring
        break;

    case Type::colours:
        break;

    case Type::mask:
        break;

    case Type::position:
        break;
    case Type::angleInc:
        break;
    case Type::lengths:
        break;
    case Type::drawRange:
        break;
    case Type::angleInit:
        break;
    }

    active = Type::null;
    imgGen.NewPreviewImageNeeded();
}

/** ****************************************************************************
 * @brief Interact::mouseMoveEvent
 * @param event
 * @param scene
 */
void Interact::mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene) {
    Q_UNUSED(scene);
    // qDebug("Mouse move    event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    if (!IsActive()) {return;}
    // Determine how much the mouse has moved while clicked
    QPointF deltaScene = event->scenePos() - pressPos;
    // deltaRatio: the movement in X & Y directions as a ratio of the scene dimensions
    QPointF deltaRatio = QPointF(deltaScene.x() / imgGen.areaSim.width(), deltaScene.y() / imgGen.areaSim.height());

    if (active == Type::arrangement) {
        // *********************************************************************
        // Arrangement edit

        // Alt disables snapping
        bool snapEn = (event->modifiers() & Qt::AltModifier) == 0;

        if (ctrlPressed) {
            // Change location
            grpActive->center = grpBackup.center + deltaScene;
            if (snapEn) {
                grpActive->center.setX(Snap(grpActive->center.x(), 9e9, imgGen.areaSim.width() * 0.05));
                grpActive->center.setY(Snap(grpActive->center.y(), 9e9, imgGen.areaSim.width() * 0.05));
            }
        }
        else {
            // Change arrangement
            switch (grpActive->type) {
            case EmType::arc: {
                // Arc: change arcRadius and arcSpan
                grpActive->arcRadius = std::max(0.0, grpBackup.arcRadius - deltaScene.y());
                qreal spanDelta = deltaRatio.x() * 3.1415926 * 2;

                grpActive->arcSpan = std::max(0.0, grpBackup.arcSpan + spanDelta);
                if (snapEn) {
                    grpActive->arcSpan = Snap(grpActive->arcSpan, PI, PI * 0.1);
                }
                break;
            }
            case EmType::line:
                // Line: change length
                grpActive->lenTotal = std::max(0.0, grpBackup.lenTotal - deltaScene.y());
                break;
            default:
                return;
            }
        }

        emit imgGen.EmitterArngmtChanged();
    }
    else if (active == Type::colours) {
        // *********************************************************************
        // Colour list edit
        // Adjust Hue
        ColourList newList = clrListBackup;
        // Change the hue of every colour in the list
        int hueOffset = -deltaRatio.y() * 359; // 359 is max scale for hue
        for (ClrFix& clrFix : newList) {
            clrFix.clr = clrFix.clr.toHsv();
            clrFix.clr.setHsv(modPos(clrFix.clr.hue() + hueOffset, 360),
                              clrFix.clr.saturation(), clrFix.clr.value());
            clrFix.clr = clrFix.clr.toRgb();
        }

        // Adjust Dynamic range (by pushing colours towards one side)
        // f(x) = x(ax + b)
        // f(1) = 1 = a + b        b = 1 - a
        // f'(0) >= 0        b >= 0
        // a: non-linearity. 0: no change. 1: maximum (to avoid negative values breaking things)
        qreal b = std::max(0., 1 - deltaRatio.x());
        qreal a = 1 - b;
        for (ClrFix& clrFix : newList) {
            const auto x = clrFix.loc;
            clrFix.loc = a * x * x + b * x;
        }

        colourMap.SetColourList(newList);
    }
    else if (active == Type::mask) {
        // *********************************************************************
        // Mask edit
        auto newMaskCfg = maskConfigBackup;
        if (ctrlPressed) {
            // X:Offset. Y: Transition width/sharpness
            newMaskCfg.offset = maskConfigBackup.offset + deltaRatio.x();
            newMaskCfg.numRevs = qBound(0.1, maskConfigBackup.numRevs + deltaRatio.y() * 5., 200.0);

        }
        else {
            // X:Ripple Count. Y:Duty Cycle
            newMaskCfg.smooth = qBound(0.0, maskConfigBackup.smooth + deltaRatio.x(), 2.0);
            newMaskCfg.dutyCycle = qBound(0.0, maskConfigBackup.dutyCycle - deltaRatio.y() * 1.0, 1.0);
        }
        imgGen.s.maskCfg = newMaskCfg;
    }
    else if (active == Type::lengths) {
        // *********************************************************************
        // Four bar lengths edit
        auto newFbCfg = fourBarBackup;
        newFbCfg.lenRatioB = newFbCfg.lenRatioB * (1 + deltaRatio.x());
        newFbCfg.lenRatio2 = newFbCfg.lenRatio2 * (1 + deltaRatio.y());
        imgGen.s.fourBar = newFbCfg;
    }
    else if (active == Type::angleInc) {
        // *********************************************************************
        // Four bar lengths edit
        auto newFbCfg = fourBarBackup;
        newFbCfg.revRatioB = fourBarBackup.revRatioB * (1 + deltaRatio.x() * 0.01);

        imgGen.s.fourBar = newFbCfg;
    }
    else if (active == Type::position) {
        // *********************************************************************
        // Four bar position edit
        auto newFbCfg = fourBarBackup;
        newFbCfg.baseSepX = qMax(0., fourBarBackup.baseSepX * (1 + deltaRatio.x()));
        newFbCfg.baseOffsetY = fourBarBackup.baseOffsetY + deltaRatio.y();

        imgGen.s.fourBar = newFbCfg;
    }
    else if (active == Type::drawRange) {
        // *********************************************************************
        // Four bar drawing range
        auto newFbCfg = fourBarBackup;
        newFbCfg.revCount = fourBarBackup.revCount * (1 + deltaRatio.x() * 1.);
        newFbCfg.initAngleOffset = fourBarBackup.initAngleOffset + deltaRatio.y() * 2. * PI;
        imgGen.s.fourBar = newFbCfg;
    }
    else if (active == Type::angleInit) {
        // *********************************************************************
        // Four bar initial angle
        auto newFbCfg = fourBarBackup;
        //newFbCfg.revCount = fourBarBackup.revCount * (1 + deltaRatio.x() * 1.);
        newFbCfg.ta1Init = fourBarBackup.ta1Init + deltaRatio.x() * 2. * PI;
        newFbCfg.initAngleOffset = fourBarBackup.initAngleOffset + deltaRatio.y() * 2. * PI;
        imgGen.s.fourBar = newFbCfg;
    }


    imgGen.NewQuickImageNeeded();
}

/** ****************************************************************************
 * @brief Interact::Cancel
 */
void Interact::Cancel() {
    // Restore the backup
    switch (active) {
    case Type::null:
        break;

    case Type::arrangement:
        if (grpActive) {
            *grpActive = grpBackup;
        }
        emit imgGen.EmitterArngmtChanged();
        break;

    case Type::colours:
        colourMap.SetColourList(clrListBackup);
        break;

    case Type::mask:
        imgGen.s.maskCfg = maskConfigBackup;
        break;

    case Type::lengths:
    case Type::angleInc:
    case Type::position:
    case Type::angleInit:
    case Type::drawRange:
        imgGen.s.fourBar = fourBarBackup;
        break;
    }
    imgGen.NewPreviewImageNeeded();
    active = Type::null;
}


/** ****************************************************************************
 * @brief Interact::KeyPressEvent
 * @param event
 */
void Interact::KeyPressEvent(QKeyEvent *event)
{
    qDebug() << "Key pressed: " << event->key();
    switch (event->key()) {
    case Qt::Key_0:
        imgGen.testVal--;
        break;
    case Qt::Key_1:
        qDebug("Interact: Lengths");
        SelectType(Type::lengths);
        break;
    case Qt::Key_2:
        qDebug("Interact: AngleInc");
        SelectType(Type::angleInc);
        break;
    case Qt::Key_3:
        qDebug("Interact: Position");
        SelectType(Type::position);
        break;
    case Qt::Key_4:
        qDebug("Interact: drawRange");
        SelectType(Type::drawRange);
        break;
    case Qt::Key_5:
        mainWindow.dumpObjectInfo();
        mainWindow.dumpObjectTree();
        break;
    case Qt::Key_Plus: // !@# temp
        imgGen.s.fourBar.temp *= 1.2;
        qDebug("imgGen.s.fourBar.temp = %.4f", imgGen.s.fourBar.temp);
        imgGen.NewImageNeeded();
        break;
    case Qt::Key_Minus: // !@# temp
        imgGen.s.fourBar.temp *= (1./1.2);
        qDebug("imgGen.s.fourBar.temp = %.4f", imgGen.s.fourBar.temp);
        imgGen.NewImageNeeded();
        break;
    case Qt::Key_Escape:
        Cancel();
        break;
    default:
        event->setAccepted(false);
    }
}
