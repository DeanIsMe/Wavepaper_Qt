#include "datatypes.h"
#include "interact.h"
#include "imagegen.h"
#include "colourmap.h"
#include <QGraphicsSceneMouseEvent>

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/
Interact interact(&imageGen); // Interaction

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
        SelectType(defaultType); // Revert to default
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
        maskConfigBackup = colourMap.GetMaskConfig();
        break;
    case Type::colours:
        // No backup required
        clrListBackup = colourMap.GetColourList();
        break;
    case Type::arrangement:
        // Interact with the emitter arrangement
        grpActive = imgGen->GetActiveArrangement();
        if (!grpActive) {
            // No active groups
            return;
        }
        grpBackup = *grpActive; // Save, so that it can be reverted
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
        emit imgGen->EmitterArngmtChanged(); // Just need to redraw the axes lines when mirroring
        break;

    case Type::colours:
        break;

    case Type::mask:
        break;

    }

    active = Type::null;
    imgGen->NewPreviewImageNeeded();
}

/** ****************************************************************************
 * @brief Interact::mouseMoveEvent
 * @param event
 * @param scene
 */
void Interact::mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene *scene) {
    Q_UNUSED(scene);
    qDebug("Mouse move    event (%7.2f, %7.2f)", event->scenePos().x(), event->scenePos().y());
    if (!IsActive()) {return;}
    // Determine how much the mouse has moved while clicked
    QPointF deltaScene = event->scenePos() - pressPos;
    // deltaRatio: the movement in X & Y directions as a ratio of the scene dimensions
    QPointF deltaRatio = QPointF(deltaScene.x() / imgGen->areaSim.width(), deltaScene.y() / imgGen->areaSim.height());

    if (active == Type::arrangement) {
        // *********************************************************************
        // Arrangement edit

        // Alt disables snapping
        bool snapEn = (event->modifiers() & Qt::AltModifier) == 0;

        if (ctrlPressed) {
            // Change location
            grpActive->center = grpBackup.center + deltaScene;
            if (snapEn) {
                grpActive->center.setX(Snap(grpActive->center.x(), 9e9, imgGen->areaSim.width() * 0.05));
                grpActive->center.setY(Snap(grpActive->center.y(), 9e9, imgGen->areaSim.width() * 0.05));
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

        emit imgGen->EmitterArngmtChanged();
    }
    else if (active == Type::colours) {
        ColourList newList = clrListBackup;
        // Change the hue of every colour in the list
        struct {
            void adjustHue(QColor & clr, int hueOffset) {
                clr = clr.toHsv();
                clr.setHsv(modPos(clr.hue() + hueOffset, 360), clr.saturation(), clr.value());
            }
        } fn;

        int hueOffset = -deltaRatio.x() * 359; // 359 is max scale for hue
        for (ClrFix& clrFix : newList) {
            fn.adjustHue(clrFix.clr, hueOffset);
        }
        colourMap.SetColourList(newList);
    }else if (active == Type::mask) {
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
        colourMap.SetMaskConfig(newMaskCfg);
    }

    imgGen->NewQuickImageNeeded();
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
        break;

    case Type::colours:
        colourMap.SetColourList(clrListBackup);
        break;

    case Type::mask:
        colourMap.SetMaskConfig(maskConfigBackup);
        break;
    }
    active = Type::null;
}
