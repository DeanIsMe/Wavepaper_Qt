#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include <QBrush>
#include <QKeyEvent>
#include "previewscene.h"
#include "imagegen.h"
#include "colourmap.h"
#include "interact.h"
#include "valueEditors.h"


/** ****************************************************************************
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , interact(*this, imageGen)
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

    QWidget * central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout * layoutCentral = new QHBoxLayout;
    central->setLayout(layoutCentral);

    // Example button column
    QVBoxLayout * layoutButtonCol = new QVBoxLayout;

    layoutCentral->addLayout(layoutButtonCol);

    imageGen.InitViewAreas();

    previewView = new PreviewView(this);
    layoutCentral->addWidget(previewView);

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


    previewScene->EmitterArngmtToList(imageGen);

    qDebug() << "Preview view rect " << RectToQString(previewView->rect());
    qDebug() << "Preview view frameRect " << RectToQString(previewView->frameRect());
    qDebug() << "Preview view sceneRect " << RectFToQString(previewView->sceneRect());

    // Generate the image
    // Use background brush
//    QImage * image = new QImage;
//    imageGen.GenerateImage(*image);
//    QBrush brush(*image);
//    previewScene->setBackgroundBrush(brush);

    previewScene->EmitterArngmtToList(imageGen);
    imageGen.NewImageNeeded();

    // Add colour map editor UI
    if (0) {
    colourMapEditor = new ColourMapEditorWidget(imageGen);
    layoutCentral->addWidget(colourMapEditor);

    QObject::connect(ui->actionShowMaskChart, QAction::toggled,
                     colourMapEditor, &ColourMapEditorWidget::SetMaskChartVisible);

    // actionMaskEnable is handled by the function on_actionMaskEnable_triggered

    QObject::connect(&interact, &Interact::InteractTypeChanged,
                     this, &MainWindow::OnInteractChange, Qt::QueuedConnection);
    }

    // Value editors
    EditorGroupWidget * valueEditors = new EditorGroupWidget(&imageGen);


    valueEditors->AddValueEditor(new ValueEditorWidget("Pos Bx", &imageGen.s.fourBar.xb, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Pos By", &imageGen.s.fourBar.yb, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Length A1", &imageGen.s.fourBar.la1, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Length A2", &imageGen.s.fourBar.la2, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Length B1", &imageGen.s.fourBar.lb1, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Length B2", &imageGen.s.fourBar.lb2, 1, 500, 0));
    valueEditors->AddValueEditor(new ValueEditorWidget("Angle increment A", &imageGen.s.fourBar.inca, 0.001, 0.1, 3));
    valueEditors->AddValueEditor(new ValueEditorWidget("Angle increment B", &imageGen.s.fourBar.incb, 0.001, 0.1, 3));
    valueEditors->AddValueEditor(new ValueEditorWidget("Init angle A", &imageGen.s.fourBar.ta1Init, -2*PI, 2*PI, 2));
    valueEditors->AddValueEditor(new ValueEditorWidget("Init angle B", &imageGen.s.fourBar.tb1Init, -2*PI, 2*PI, 2));

    QScrollArea * valueEditorsScroll = new QScrollArea();
    valueEditorsScroll->setWidget(valueEditors);
    valueEditorsScroll->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    layoutCentral->addWidget(valueEditorsScroll);

    // Text window for debugging
    layoutCentral->addWidget(textWindow);
}

/** ****************************************************************************
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
    delete ui;
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


