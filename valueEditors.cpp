#include "valueEditors.h"
#include <QLabel>
#include "imagegen.h"

/** ****************************************************************************
 * @brief SliderSpinEditor::SliderSpinEditor
 * @param numberIn
 */
SliderSpinEditor::SliderSpinEditor(QString name, qreal *numberIn, qreal minIn, qreal maxIn, int precisionIn) :
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
    QObject::connect(&spinBox, &QAbstractSpinBox::editingFinished, this, &SliderSpinEditor::SpinChangedSlot);

    layout->addWidget(&slider);
    slider.setOrientation(Qt::Horizontal);
    QObject::connect(&slider, &QSlider::valueChanged, this, &SliderSpinEditor::SliderChangedSlot);
    QObject::connect(&slider, &QSlider::sliderReleased, this, &SliderSpinEditor::SliderChangedSlot);
    parentGroupWidget = nullptr;
}

/** ****************************************************************************
 * @brief SliderSpinEditor::~SliderSpinEditor
 */
SliderSpinEditor::~SliderSpinEditor()
{
    if (parentGroupWidget != nullptr) {
        parentGroupWidget->RemoveValueEditor(this);
    }
}

/** ****************************************************************************
 * @brief SliderSpinEditor::SpinChangedSlot
 */
void SliderSpinEditor::SpinChangedSlot()
{
    *extValue = spinBox.value();
    slider.blockSignals(true);
    slider.setValue(*extValue * sliderScaler);
    slider.blockSignals(false);
    emit ValueEditedSig();
}

/** ****************************************************************************
 * @brief SliderSpinEditor::SliderChangedSlot
 */
void SliderSpinEditor::SliderChangedSlot()
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
 * @brief SliderSpinEditor::ApplyExtValue pulls the external value and applies
 * it to this widget
 */
void SliderSpinEditor::ApplyExtValue()
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
SliderSpinEditor * EditorGroupWidget::AddValueEditor(SliderSpinEditor *valEditWidget)
{
    valueEditors.append(valEditWidget);
    layout.addWidget(valEditWidget);
    valEditWidget->SetParentGroupWidget(this);

    // Connect signals and slots for this
    QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedSig, imgGen, &ImageGen::NewImageNeeded);
    QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedQuickSig, imgGen, &ImageGen::NewQuickImageNeeded);
    // This is inefficient - should use a smarter method of determining when to update the values (perhaps)
    QObject::connect(imgGen, &ImageGen::GenerateImageSignal, valEditWidget, &SliderSpinEditor::ApplyExtValue);

    return valEditWidget;
}

/** ****************************************************************************
 * @brief EditorGroupWidget::RemoveValueEditor removes the given value editor
 * from the list. Does NOT delete the object.
 * @param valEditWidget
 * @returns true if the value was removed
 */
bool EditorGroupWidget::RemoveValueEditor(SliderSpinEditor *valEditWidget)
{
    layout.removeWidget(valEditWidget);
    return valueEditors.removeOne(valEditWidget);
//    for (int i = 0; i < valueEditors.length(); i++) {
//        if (valEditWidget == valueEditors[i]) {
//            // Remove this widget
//        }
//    }
}
