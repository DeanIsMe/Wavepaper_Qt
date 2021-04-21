#ifndef INTERACT_H
#define INTERACT_H

#include "datatypes.h"
#include <QGraphicsView>
class ColourMap;

/** ****************************************************************************
 * @brief The Interact class
 */
class Interact : public QObject {
    Q_OBJECT
public:
    enum Type : int {
        null,
        arrangement, // Length/radius + rotation
        arrangement2,// Count (+ span for arc)
        location, // Group center
        wavelength, // + linearity
        colours,
        mask,
        // Four bar linkage
        lengths,
        angleInc,
        position,
        drawRange,
        angleInit, // Unused
    };
    static constexpr Type defaultTypeWaves = Type::arrangement;
    static constexpr Type defaultTypeFourBar = Type::lengths;

private:

    MainWindow& mainWindow;
    ImageGen & imgGen;
    ColourMap & colourMap;
    Type active = Type::null; // Saves the active interact type upon mouse press.
    Type typeSelected = defaultTypeWaves;

    QPointF pressPos; // Where the interaction started
    bool ctrlPressed; // If the 'control' key was pressed at the start of the interaction
    // For emitter arrangement changes:
    EmArrangement grpBackup;
    EmArrangement * grpActive;
    qreal wavelengthBackup;
    qreal distOffsetBackup;
    // For mask changes
    MaskCfg maskConfigBackup;
    // For colour list changes
    ColourList clrListBackup;
    FourBarCfg fourBarBackup;

private:
    void Cancel();
    void SelectDefaultType();

public:
    Interact(MainWindow& mainWindowIn, ImageGen& imgGenIn);
    void mousePressEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    bool IsActive() {return active != Type::null;}
    bool TypeIsSelected(Type type) {return typeSelected == type;}
    Type GetTypeSelected() {return typeSelected;}
    bool TypeIsActive(Type type) {return active == type;} // Active == currently pressed
    void SelectType(Type type);
    void DeselectType(Type type);
    void SetTypeSelect(Type type, bool en) {if (en) SelectType(type);
                                           else DeselectType(type);}
    EmArrangement * GetActiveArrangement() { return grpActive; }
    void KeyPressEvent(QKeyEvent * event);

signals:
    void InteractTypeChanged(QVariant type);
};

#endif // INTERACT_H
