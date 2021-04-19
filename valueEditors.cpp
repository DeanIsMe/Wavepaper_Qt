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

SliderSpinEditor::SliderSpinEditor(QString name, qint32 *numberIn, qreal minIn, qreal maxIn, int precisionIn) :
    extValue(nullptr), extValueInt(numberIn), minVal(minIn), maxVal(maxIn), precision(precisionIn)
{
    ConstructorSub(name);
}

void SliderSpinEditor::ConstructorSub(const QString& name)
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
    SetExtVal((qreal)spinBox.value());
    slider.blockSignals(true);
    slider.setValue(GetExtVal() * sliderScaler);
    slider.blockSignals(false);
    emit ValueEditedSig();
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
 * @param valRedrawsOverlay is true if changing of the value necessitates a
 * re-drawing of the emitters
 * @return
 */
SliderSpinEditor * EditorGroupWidget::AddValueEditor(SliderSpinEditor *valEditWidget, bool valRedrawsOverlay)
{
    valueEditors.append(valEditWidget);
    layout.addWidget(valEditWidget);
    valEditWidget->SetParentGroupWidget(this);

    // Connect signals and slots for this
    QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedSig, imgGen, &ImageGen::NewImageNeeded);
    QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedQuickSig, imgGen, &ImageGen::NewQuickImageNeeded);
    // This is inefficient - but it's not called that frequently so it should be fine
    QObject::connect(imgGen, &ImageGen::GenerateImageSignal, valEditWidget, &SliderSpinEditor::ApplyExtValue);

    if (valRedrawsOverlay) {
        QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedSig, imgGen, &ImageGen::EmitterArngmtChanged);
        QObject::connect(valEditWidget, &SliderSpinEditor::ValueEditedQuickSig, imgGen, &ImageGen::EmitterArngmtChanged);
    }
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
