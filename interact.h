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
        arrangement,
        colours,
        mask,
    };

private:
    static constexpr Type defaultType = Type::arrangement;

    ImageGen & imgGen;
    ColourMap & colourMap;
    Type active = Type::null; // Saves the active interact type upon mouse press.
    Type typeSelected = defaultType;

    QPointF pressPos; // Where the interaction started
    bool ctrlPressed; // If the 'control' key was pressed at the start of the interaction
    // For emitter arrangement changes:
    EmArrangement grpBackup;
    EmArrangement * grpActive;
    // For mask changes
    MaskCfg maskConfigBackup;
    // For colour list changes
    ColourList clrListBackup;

private:
    void Cancel();

public:
    Interact(ImageGen& imgGenIn);
    void mousePressEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, PreviewScene * scene);
    bool IsActive() {return active != Type::null;}
    bool TypeIsSelected(Type type) {return typeSelected == type;}
    bool TypeIsActive(Type type) {return active == type;}
    void SelectType(Type type);
    void DeselectType(Type type);
    void SetTypeSelect(Type type, bool en) {if (en) SelectType(type);
                                           else DeselectType(type);}
    EmArrangement * GetActiveArrangement() { return grpActive; }

signals:
    void InteractTypeChanged(QVariant type);
};

extern Interact interact; // Interaction

#endif // INTERACT_H
