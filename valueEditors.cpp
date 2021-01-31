#include "valueEditors.h"
#include <QLabel>
#include "imagegen.h"

/** ****************************************************************************
 * @brief ValueEditorWidget::ValueEditorWidget
 * @param numberIn
 */
ValueEditorWidget::ValueEditorWidget(QString name, qreal *numberIn, qreal minIn, qreal maxIn, int precisionIn) :
    extValue(numberIn), minVal(minIn), maxVal(maxIn), precision(precisionIn)
{
    //this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    this->setMinimumWidth(200);
    this->setMaximumHeight(80);

    QVBoxLayout * layout = new QVBoxLayout();
    this->setLayout(layout);

    QLabel * labelName = new QLabel();
    labelName->setText(name);
    layout->addWidget(labelName);

    ApplyExtValue();

    spinBox.setMaximum(maxVal);
    spinBox.setMinimum(minVal);
    spinBox.setDecimals(precision);

    sliderScaler = pow(10,precision);
    slider.setMinimum(minVal * sliderScaler);
    slider.setMaximum(maxVal * sliderScaler);

    layout->addWidget(&spinBox);
    QObject::connect(&spinBox, &QAbstractSpinBox::editingFinished, this, &ValueEditorWidget::SpinChangedSlot);

    layout->addWidget(&slider);
    slider.setOrientation(Qt::Horizontal);
    QObject::connect(&slider, &QSlider::valueChanged, this, &ValueEditorWidget::SliderChangedSlot);
}

/** ****************************************************************************
 * @brief ValueEditorWidget::SpinChangedSlot
 * @param newVal
 */
void ValueEditorWidget::SpinChangedSlot()
{
    *extValue = spinBox.value();
    slider.blockSignals(true);
    slider.setValue(*extValue * sliderScaler);
    slider.blockSignals(false);
    emit ValueEditedSig();
}

/** ****************************************************************************
 * @brief ValueEditorWidget::SliderChangedSlot
 * @param newSliderVal
 */
void ValueEditorWidget::SliderChangedSlot(int newSliderVal)
{
    *extValue = (qreal)newSliderVal / sliderScaler;
    spinBox.blockSignals(true);
    spinBox.setValue(*extValue);
    spinBox.blockSignals(false);
    if (slider.isSliderDown()) {
        // User is currently dragging the slider
        emit ValueEditedQuickSig();
    }
    else {
        emit ValueEditedSig();
    }
}

/** ****************************************************************************
 * @brief ValueEditorWidget::ApplyExtValue pulls the external value and applies
 * it to this widget
 */
void ValueEditorWidget::ApplyExtValue()
{
    spinBox.blockSignals(true);
    spinBox.setValue(*extValue);
    spinBox.blockSignals(false);

    slider.blockSignals(true);
    slider.setValue(*extValue * sliderScaler);
    slider.blockSignals(false);
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/

/** ****************************************************************************
 * @brief EditorGroupWidget::EditorGroupWidget
 */
EditorGroupWidget::EditorGroupWidget(ImageGen * imgGenIn) : imgGen(imgGenIn)
{
    this->setLayout(&layout);
}

/** ****************************************************************************
 * @brief EditorGroupWidget::~EditorGroupWidget
 */
EditorGroupWidget::~EditorGroupWidget()
{
    qDeleteAll(valueEditors);
    valueEditors.clear();
}

/** ****************************************************************************
 * @brief EditorGroupWidget::AddValueEditor
 * @param valEditWidget
 * @return
 */
ValueEditorWidget * EditorGroupWidget::AddValueEditor(ValueEditorWidget *valEditWidget)
{
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    valueEditors.append(valEditWidget);
    layout.addWidget(valEditWidget);

    // Connect signals and slots for this
    QObject::connect(valEditWidget, &ValueEditorWidget::ValueEditedSig, imgGen, &ImageGen::NewImageNeeded);
    QObject::connect(valEditWidget, &ValueEditorWidget::ValueEditedQuickSig, imgGen, &ImageGen::NewQuickImageNeeded);
    // This is inefficient - should use a smarter method of determining when to update the values (perhaps)
    QObject::connect(imgGen, &ImageGen::GenerateImageSignal, valEditWidget, &ValueEditorWidget::ApplyExtValue);

    return valEditWidget;
}
