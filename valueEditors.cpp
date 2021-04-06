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
    //this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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
    QObject::connect(&slider, &QSlider::sliderReleased, this, &ValueEditorWidget::SliderChangedSlot);
    parentGroupWidget = nullptr;
}

/** ****************************************************************************
 * @brief ValueEditorWidget::~ValueEditorWidget
 */
ValueEditorWidget::~ValueEditorWidget()
{
    if (parentGroupWidget != nullptr) {
        parentGroupWidget->RemoveValueEditor(this);
    }
}

/** ****************************************************************************
 * @brief ValueEditorWidget::SpinChangedSlot
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
 */
void ValueEditorWidget::SliderChangedSlot()
{
    *extValue = (qreal)slider.value() / sliderScaler;
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
 * @brief EditorGroupWidget::ClearAllValueEditors
 */
void EditorGroupWidget::ClearAllValueEditors()
{
    for (qint32 i = 0; i < valueEditors.length(); i++) {
        layout.removeWidget(valueEditors[i]);
    }
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
    valueEditors.append(valEditWidget);
    layout.addWidget(valEditWidget);
    valEditWidget->SetParentGroupWidget(this);

    // Connect signals and slots for this
    QObject::connect(valEditWidget, &ValueEditorWidget::ValueEditedSig, imgGen, &ImageGen::NewImageNeeded);
    QObject::connect(valEditWidget, &ValueEditorWidget::ValueEditedQuickSig, imgGen, &ImageGen::NewQuickImageNeeded);
    // This is inefficient - should use a smarter method of determining when to update the values (perhaps)
    QObject::connect(imgGen, &ImageGen::GenerateImageSignal, valEditWidget, &ValueEditorWidget::ApplyExtValue);

    return valEditWidget;
}

/** ****************************************************************************
 * @brief EditorGroupWidget::RemoveValueEditor removes the given value editor
 * from the list. Does NOT delete the object.
 * @param valEditWidget
 * @returns true if the value was removed
 */
bool EditorGroupWidget::RemoveValueEditor(ValueEditorWidget *valEditWidget)
{
    layout.removeWidget(valEditWidget);
    return valueEditors.removeOne(valEditWidget);
//    for (int i = 0; i < valueEditors.length(); i++) {
//        if (valEditWidget == valueEditors[i]) {
//            // Remove this widget
//        }
//    }
}
