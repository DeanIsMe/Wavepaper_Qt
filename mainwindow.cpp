 #include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QPushButton>
#include <QBrush>
#include <QKeyEvent>
#include "previewscene.h"
#include "imagegen.h"
#include "interact.h"
#include "valueEditors.h"


/** ****************************************************************************
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , interact(*this, imageGen)
    , programMode(ProgramMode::waves)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Add a text window for debugging info
    textWindow = new QPlainTextEdit;
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    //textWindow->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textWindow->setFont(font);

    imageGen.SetMainWindow(this);

    imageGen.s.emitterRadius = 2.0;

    setCentralWidget(&centralWidget);

    centralWidget.setLayout(&layoutCentral);

    // Example button column
    QVBoxLayout * layoutButtonCol = new QVBoxLayout;

    layoutCentral.addLayout(layoutButtonCol);

    imageGen.InitViewAreas();

    previewView = new PreviewView(this);
    layoutCentral.addWidget(previewView);

    previewScene = new PreviewScene(previewView, *this);
    previewScene->setSceneRect(imageGen.areaSim);
    previewView->setScene(previewScene);

    previewView->setSceneRect(imageGen.areaSim);

    QObject::connect(&imageGen, &ImageGen::NewImageReady,
                     previewView, &PreviewView::OnPatternImageChange);

    QObject::connect(&imageGen, &ImageGen::OverlayTextSignal,
                     previewScene, &PreviewScene::OverlayTextSlot);

    QObject::connect(&imageGen, &ImageGen::EmitterArngmtChanged,
                     previewScene, &PreviewScene::OnEmitterArngmtChange,
                     Qt::QueuedConnection);


    QObject::connect(&interact, &Interact::InteractTypeChanged,
                     this, &MainWindow::OnInteractChange, Qt::QueuedConnection);

    // Action trigger events
    QObject::connect(ui->actionFewer, QAction::triggered,
                     &imageGen, &ImageGen::EmitterCountDecrease);
    QObject::connect(ui->actionMore, QAction::triggered,
                     &imageGen, &ImageGen::EmitterCountIncrease);

    QObject::connect(ui->actionWavelengthDecrease, QAction::triggered,
                     &imageGen, &ImageGen::WavelengthDecrease);

    QObject::connect(ui->actionWavelengthIncrease, QAction::triggered,
                     &imageGen, &ImageGen::WavelengthIncrease);

    QObject::connect(ui->actionHideEmitters, QAction::toggled,
                     &imageGen, &ImageGen::HideEmitters);

    QObject::connect(ui->actionSaveImage, QAction::triggered,
                     &imageGen, &ImageGen::SaveImage);

    QObject::connect(ui->actionWaveMode, QAction::triggered,
                     this, &MainWindow::ChangeModeToWaves);

    QObject::connect(ui->actionFourBarMode, QAction::triggered,
                     this, &MainWindow::ChangeModeToFourBar);

    previewScene->EmitterArngmtToList(imageGen);

    qDebug() << "Preview view rect " << RectToQString(previewView->rect());
    qDebug() << "Preview view frameRect " << RectToQString(previewView->frameRect());
    qDebug() << "Preview view sceneRect " << RectFToQString(previewView->sceneRect());

    previewScene->EmitterArngmtToList(imageGen);
    imageGen.NewImageNeeded();

    // Value editors
    valueEditorWidget = new EditorGroupWidget(&imageGen);
    QScrollArea * valueEditorScroll = new QScrollArea();
    valueEditorScroll->setWidget(valueEditorWidget);
    valueEditorScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    valueEditorScroll->setWidgetResizable(false); // The default
    layoutCentral.addWidget(valueEditorScroll);

    // Text window for debugging
    layoutCentral.addWidget(textWindow);

    InitMode();
}

/** ****************************************************************************
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/** ****************************************************************************
 * @brief MainWindow::InitMode
 */
void MainWindow::InitMode()
{
    qDebug("Init mode %d", (int)programMode);

    if (colourMapEditor != nullptr) {
        layoutCentral.removeWidget(colourMapEditor);
        colourMapEditor->setVisible(false);
    }

    // Add colour map editor UI
    if (programMode == ProgramMode::waves) {
        if (colourMapEditor == nullptr) {
            colourMapEditor = new ColourMapEditorWidget(imageGen);
            QObject::connect(ui->actionShowMaskChart, QAction::toggled,
                             colourMapEditor, &ColourMapEditorWidget::SetMaskChartVisible);
        }
        layoutCentral.addWidget(colourMapEditor);
        colourMapEditor->setVisible(true);
    }

    // Value editors
    valueEditorWidget->ClearAllValueEditors();

    if (programMode == ProgramMode::waves) {
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Wavelength", &imageGen.s.wavelength, 1, 200, 1));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Linearity", &imageGen.s.distOffsetF, 0, 1.0, 2));

        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Mask Revolutions", &imageGen.s.maskCfg.numRevs, 1, 80, 0));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Mask Offset", &imageGen.s.maskCfg.offset, 0, 1, 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Mask Duty Cycle", &imageGen.s.maskCfg.dutyCycle, 0, 1, 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Mask Smooth", &imageGen.s.maskCfg.smooth, 0, 2.0, 2));

    }
    if (programMode == ProgramMode::fourBar) {
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Base separation X", &imageGen.s.fourBar.baseSepX, 0, 5, 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Base offset Y", &imageGen.s.fourBar.baseOffsetY, -5, 5, 2));

        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Length ratio side", &imageGen.s.fourBar.lenRatioB, 0.1, 10, 3));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Length ratio apex", &imageGen.s.fourBar.lenRatio2, 0.1, 10, 3));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Length scale", &imageGen.s.fourBar.lenBase, 1, 200, 0));

        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Init angle A", &imageGen.s.fourBar.ta1Init, -2*PI, 2*PI, 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Init angle offset", &imageGen.s.fourBar.initAngleOffset, -2*PI, 2*PI, 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Revolution count", &imageGen.s.fourBar.revCount, 1, 1000, 0));

        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Revolution ratio B", &imageGen.s.fourBar.revRatioB, 0, 1000, 4));

        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Line width", &imageGen.s.fourBar.lineWidth, 0., 10., 2));
        valueEditorWidget->AddValueEditor(new ValueEditorWidget("Line taper ratio", &imageGen.s.fourBar.lineTaperRatio, 0., 1., 2));
    }

    // Interact mode
    if (programMode == ProgramMode::waves) {
        interact.SelectType(Interact::defaultTypeWaves);
    }
    else if (programMode == ProgramMode::fourBar) {
        interact.SelectType(Interact::defaultTypeFourBar);
    }

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents ); // https://stackoverflow.com/a/30472749/3580080
    valueEditorWidget->resize(valueEditorWidget->sizeHint());
    centralWidget.resize(centralWidget.sizeHint());


    // Update the toolbar
    ui->toolBarHorz->clear();

    QList<QAction *> actionsToAdd;
    actionsToAdd.append(ui->actionSaveImage);
    actionsToAdd.append(ui->actionWaveMode);
    actionsToAdd.append(ui->actionFourBarMode);



    if (programMode == ProgramMode::waves) {
        actionsToAdd.append(ui->actionEditGroup);
        actionsToAdd.append(ui->actionHideEmitters);
        actionsToAdd.append(ui->actionMore);
        actionsToAdd.append(ui->actionFewer);
        actionsToAdd.append(ui->actionMirrorHor);
        actionsToAdd.append(ui->actionMirrorVert);
        actionsToAdd.append(ui->actionWavelengthDecrease);
        actionsToAdd.append(ui->actionWavelengthIncrease);
        actionsToAdd.append(ui->actionMaskEnable);
        actionsToAdd.append(ui->actionMaskEdit);
        actionsToAdd.append(ui->actionShowMaskChart);
        actionsToAdd.append(ui->actionColoursEdit);
    }
    if (programMode == ProgramMode::fourBar) {
        actionsToAdd.append(ui->actionFbEditLengths);
        actionsToAdd.append(ui->actionFbEditAngleInc);
        actionsToAdd.append(ui->actionFbEditDrawRange);
    }

    ui->toolBarHorz->addActions(actionsToAdd);

    ui->actionWaveMode->setChecked(programMode == ProgramMode::waves);
    ui->actionFourBarMode->setChecked(programMode == ProgramMode::fourBar);





}

/** ****************************************************************************
 * @brief MainWindow::keyPressEvent
 * @param event
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    return interact.KeyPressEvent(event);
}

/** ****************************************************************************
 * @brief PaintTestImage
 * @param testCanvas
 */
void PaintTestImage(QWidget * testCanvas) {
    QRect testArea = testCanvas->rect();
    QPainter painter(testCanvas);
    //painter.setWindow(testArea);

    QImage imageTest(testArea.size(), QImage::Format_ARGB32);
    for (int y = 0; y < imageTest.height(); y++) {
        QColor clr;
        switch (y % 3) {
        case 0: clr = Qt::green; break;
        case 1: clr = Qt::red; break;
        case 2: clr = Qt::white; break;
        }
        for (int x = 0; x < imageTest.width(); x++) {
            imageTest.setPixelColor(x, y, clr);
        }
    }
    painter.drawImage(testArea.x(), testArea.y(), imageTest);
}

/** ****************************************************************************
 * @brief MainWindow::on_actionMirrorHor_triggered
 * @param checked
 */
void MainWindow::on_actionMirrorHor_triggered(bool checked)
{
    imageGen.GetActiveArrangement()->mirrorHor = checked;
    previewScene->EmitterArngmtToList(imageGen);
    imageGen.NewImageNeeded();
}

/** ****************************************************************************
 * @brief MainWindow::on_actionMirrorVert_triggered
 * @param checked
 */
void MainWindow::on_actionMirrorVert_triggered(bool checked)
{
    imageGen.GetActiveArrangement()->mirrorVert = checked;
    previewScene->EmitterArngmtToList(imageGen);
    imageGen.NewImageNeeded();
}

/** ****************************************************************************
 * @brief MainWindow::on_actionMaskEnable_triggered
 * @param checked
 */
void MainWindow::on_actionMaskEnable_triggered(bool checked)
{
    if (checked != imageGen.s.maskCfg.enabled) {
        imageGen.s.maskCfg.enabled = checked;
        imageGen.NewPreviewImageNeeded();
    }
    // Change checkboxes
    if (!checked) {
        ui->actionShowMaskChart->setChecked(false);
    }
    ui->actionMaskEdit->setChecked(checked);
}

/** ****************************************************************************
 * @brief MainWindow::OnInteractChange slot is called when the interaction type
 * is changed. It sets the UI buttons accordingly
 * @param type
 */
void MainWindow::OnInteractChange(QVariant interactType) {
    ui->actionEditGroup->setChecked(interactType == (Interact::Type::arrangement));
    ui->actionColoursEdit->setChecked(interactType == (Interact::Type::colours));
    ui->actionMaskEdit->setChecked(interactType == (Interact::Type::mask));

    ui->actionFbEditLengths->setChecked(interactType == Interact::Type::lengths);
    ui->actionFbEditAngleInc->setChecked(interactType == Interact::Type::angleInc);
    ui->actionFbEditDrawRange->setChecked(interactType == Interact::Type::drawRange);
}

/** ****************************************************************************
 * @brief MainWindow::ChangeModeToWaves
 */
void MainWindow::ChangeModeToWaves()
{
    if (programMode != ProgramMode::waves) {
        programMode = ProgramMode::waves;
        InitMode();
        imageGen.NewImageNeeded();
    }
}

void MainWindow::ChangeModeToFourBar()
{
    if (programMode != ProgramMode::fourBar) {
        programMode = ProgramMode::fourBar;
        InitMode();
        imageGen.NewImageNeeded();
    }
}

// Note: the 'triggered' signal is NOT sent when the value is changed with 'setChecked()'.
//       So, I use 'toggled' instead
void MainWindow::on_actionMaskEdit_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::mask, arg1);
}

void MainWindow::on_actionColoursEdit_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::colours, arg1);
}

void MainWindow::on_actionEditGroup_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::arrangement, arg1);
}

void MainWindow::on_actionHideEmitters_toggled(bool unused) {
    Q_UNUSED(unused);
}

void MainWindow::on_actionFbEditLengths_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::lengths, arg1);
}

void MainWindow::on_actionFbEditAngleInc_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::angleInc, arg1);
}

void MainWindow::on_actionFbEditDrawRange_toggled(bool arg1)
{
    interact.SetTypeSelect(Interact::Type::drawRange, arg1);
}
