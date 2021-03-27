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

/** ****************************************************************************
 * @brief The ValueEditorWidget class manages GUI editing for a single value
 * It contains a slider & a spin box, which are
 * both tied to some external qreal value. When the number is edited from one
 * of these widgets, a signal is emitted 'ValueEditedSig'.
 * When the value is changed externally, the external program should call 'ApplyValue'
 * to update the widget values.
 */
class ValueEditorWidget : public QWidget {
    Q_OBJECT;
public:
    ValueEditorWidget(QString name, qreal* numberIn, qreal minIn, qreal maxIn, int precisionIn = 0);
private:
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
 * @brief The EditorGroupWidget class contains a group of
 */
class EditorGroupWidget : public QWidget
{
    Q_OBJECT
public:
    EditorGroupWidget(ImageGen * imgGenIn);
    ~EditorGroupWidget();
    ValueEditorWidget *AddValueEditor(ValueEditorWidget * valEditWidget);

private:
    ImageGen * imgGen;

    //void (*widgetEditedValueSlot)(void);
    void (*valueChangedSignal)(void);
    QVBoxLayout layout;
    QList<ValueEditorWidget*> valueEditors;

};

#endif // VALUEEDITORS_H
