#ifndef VALUEEDITORS_H
#define VALUEEDITORS_H

#include "datatypes.h"
#include <QObject>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QList>
#include <QScrollArea>

class EditorGroupWidget;

/** ****************************************************************************
 * @brief The SliderSpinEditor class manages GUI editing for a single value
 * It contains a slider & a spin box, which are
 * both tied to some external qreal value. When the number is edited from one
 * of these widgets, a signal is emitted 'ValueEditedSig'.
 * When the value is changed externally, the external program should call 'ApplyValue'
 * to update the widget values.
 */
class SliderSpinEditor : public QWidget {
    Q_OBJECT;
public:
    SliderSpinEditor(QString name, qreal* numberIn, qreal minIn, qreal maxIn, int precisionIn = 0);
    ~SliderSpinEditor();
    void SetParentGroupWidget(EditorGroupWidget * ptrIn) {parentGroupWidget = ptrIn;}
private:
    EditorGroupWidget * parentGroupWidget;
    qreal * const extValue; // External value that these widgets edit
    qreal minVal;
    qreal maxVal;
    qreal sliderScaler; // value = sliderVal / sliderDecPlaces (because slider only supports int)
    int precision;

    QDoubleSpinBox spinBox;
    QSlider slider;
signals:
    ValueEditedSig();
    ValueEditedQuickSig();
private slots:
    void SpinChangedSlot();
    void SliderChangedSlot();
public slots:
    void ApplyExtValue();
};

/** ****************************************************************************
 * @brief The EditorGroupWidget class contains a group of SliderSpinEditor
 */
class EditorGroupWidget : public QWidget
{
    Q_OBJECT
public:
    EditorGroupWidget(ImageGen * imgGenIn);
    ~EditorGroupWidget();
    SliderSpinEditor *AddValueEditor(SliderSpinEditor * valEditWidget);
    bool RemoveValueEditor(SliderSpinEditor * valEditWidget);
    void ClearAllValueEditors();

private:
    ImageGen * imgGen;

    //void (*widgetEditedValueSlot)(void);
    void (*valueChangedSignal)(void);
    QVBoxLayout layout;
    QList<SliderSpinEditor*> valueEditors;

};

#endif // VALUEEDITORS_H
