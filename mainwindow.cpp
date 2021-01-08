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

/** ****************************************************************************
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Add a text window for debugging info
    textWindow = new QPlainTextEdit;

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

    previewScene = new PreviewScene(previewView);
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

    // Add colour map UI
    colourMapEditor = new ColourMapEditorWidget(imageGen);
    layoutCentral->addWidget(colourMapEditor);

    QObject::connect(ui->actionShowMaskChart, QAction::toggled,
                     colourMapEditor, &ColourMapEditorWidget::SetMaskChartVisible);

    // actionMaskEnable is handled by the funtion on_actionMaskEnable_triggered

    QObject::connect(&interact, &Interact::InteractTypeChanged,
                     this, &MainWindow::OnInteractChange, Qt::QueuedConnection);

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
    qDebug() << "Key pressed: " << event->key();
    switch (event->key()) {
    case Qt::Key_0:
        imageGen.testVal--;
        textWindow->appendPlainText(QString::asprintf("TestVal=%4d", imageGen.testVal));
        break;
    case Qt::Key_1:
        imageGen.testVal++;
        textWindow->appendPlainText(QString::asprintf("TestVal=%4d", imageGen.testVal));
        break;
    case Qt::Key_2:
        previewScene->OverlayTextSlot("Text Key 2");
        break;
    case Qt::Key_3:
        previewScene->OverlayTextSlot(QString::asprintf("Rendering image %.1fM pixels for %d emitters.",
                                                         2000000 / 1000000., 15));
        break;
    case Qt::Key_4:
        previewScene->OverlayTextSlot(QString());
        break;
    case Qt::Key_Plus: // !@# temp
        imageGen.s.distOffsetF *= 1.2;
        qDebug("distOffsetF = %.2f", imageGen.s.distOffsetF);
        imageGen.NewImageNeeded();
        break;
    case Qt::Key_Minus: // !@# temp
        imageGen.s.distOffsetF *= (1./1.2);
        qDebug("distOffsetF = %.2f", imageGen.s.distOffsetF);
        imageGen.NewImageNeeded();
        break;
    default:
        event->setAccepted(false);
    }
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


