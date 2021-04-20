#include "valueEditors.h"
#include <QLabel>
#include "imagegen.h"

/** ****************************************************************************
 * @brief SliderSpinEditor::SliderSpinEditor
 * @param numberIn
 */
SliderSpinEditor::SliderSpinEditor(QString name, qreal *numberIn, qreal minIn, qreal maxIn, int precisionIn) :
    extValue(numberIn), extValueInt(nullptr), minVal(minIn), maxVal(maxIn), precision(precisionIn)
{
    ConstructorSub(name);
}

SliderSpinEditor::SliderSpinEditor(QString name, qint32 *numberIn, qreal minIn, qreal maxIn) :
    extValue(nullptr), extValueInt(numberIn), minVal(minIn), maxVal(maxIn), precision(0)
{
    ConstructorSub(name);
}

void SliderSpinEditor::ConstructorSub(const QString& name)
{
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
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
    SetExtVal((qreal)spinBox.value());
    slider.blockSignals(true);
    slider.setValue(GetExtVal() * sliderScaler);
    slider.blockSignals(false);
    parentGroupWidget->InternalValueEdited();
}

/** ****************************************************************************
 * @brief SliderSpinEditor::SliderChangedSlot
 */
void SliderSpinEditor::SliderChangedSlot()
{
    SetExtVal((qreal)slider.value() / sliderScaler);
    spinBox.blockSignals(true);
    spinBox.setValue(GetExtVal());
    spinBox.blockSignals(false);
    if (slider.isSliderDown()) {
        // User is currently dragging the slider
        parentGroupWidget->InternalValueEditedQuick();
    }
    else {
        parentGroupWidget->InternalValueEdited();
    }
}


/** ****************************************************************************
 * @brief SliderSpinEditor::ApplyExtValue pulls the external value and applies
 * it to this widget
 */
void SliderSpinEditor::ApplyExtValue()
{
    spinBox.blockSignals(true);
    spinBox.setValue(GetExtVal());
    spinBox.blockSignals(false);

    slider.blockSignals(true);
    slider.setValue(GetExtVal() * sliderScaler);
    slider.blockSignals(false);
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/

/** ****************************************************************************
 * @brief EditorGroupWidget::EditorGroupWidget
 */
ValueEditorGroupWidget::ValueEditorGroupWidget()
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    this->setLayout(&layout);
}

/** ****************************************************************************
 * @brief EditorGroupWidget::~EditorGroupWidget
 */
ValueEditorGroupWidget::~ValueEditorGroupWidget()
{
    qDeleteAll(valueEditors);
    valueEditors.clear();
}

/** ****************************************************************************
 * @brief EditorGroupWidget::ClearAllValueEditors
 */
void ValueEditorGroupWidget::ClearAllValueEditors()
{
    for (qint32 i = 0; i < valueEditors.length(); i++) {
        layout.removeWidget(valueEditors[i]);
    }
    qDeleteAll(valueEditors);
    valueEditors.clear();
}

/** ****************************************************************************
 * @brief EditorGroupWidget::ApplyExternalValues
 */
void ValueEditorGroupWidget::ApplyExternalValues()
{
    for (auto& editor : valueEditors) {
        editor->ApplyExtValue();
    }
}

/** ****************************************************************************
 * @brief EditorGroupWidget::InternalValueEdited
 */
void ValueEditorGroupWidget::InternalValueEdited()
{
    prevEditSignalWasQuick = false;
    emit ValueEditedSig();
}

void ValueEditorGroupWidget::InternalValueEditedQuick()
{
    prevEditSignalWasQuick = true;
    emit ValueEditedQuickSig();
}

/** ****************************************************************************
 * @brief EditorGroupWidget::AddValueEditor
 * @param valEditWidget
 * @param valRedrawsOverlay is true if changing of the value necessitates a
 * re-drawing of the emitters
 * @return
 */
SliderSpinEditor * ValueEditorGroupWidget::AddValueEditor(SliderSpinEditor *valEditWidget)
{
    valueEditors.append(valEditWidget);
    layout.addWidget(valEditWidget);
    valEditWidget->SetParentGroupWidget(this);

    return valEditWidget;
}

/** ****************************************************************************
 * @brief EditorGroupWidget::RemoveValueEditor removes the given value editor
 * from the list. Does NOT delete the object.
 * @param valEditWidget
 * @returns true if the value was removed
 */
bool ValueEditorGroupWidget::RemoveValueEditor(SliderSpinEditor *valEditWidget)
{
    layout.removeWidget(valEditWidget);
    return valueEditors.removeOne(valEditWidget);
//    for (int i = 0; i < valueEditors.length(); i++) {
//        if (valEditWidget == valueEditors[i]) {
//            // Remove this widget
//        }
//    }
}
