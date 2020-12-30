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

    QObject::connect(&imageGen, &ImageGen::EmitterArngmtChanged,
                     previewScene, &PreviewScene::OnEmitterArngmtChange,
                     Qt::QueuedConnection);

    QObject::connect(&colourMap, &ColourMap::NewClrMapReady,
                     &imageGen, ImageGen::NewQuickImageNeeded, Qt::DirectConnection);

    // Action trigger events
    QObject::connect(ui->actionFewer, QAction::triggered,
                     &imageGen, &ImageGen::EmitterCountDecrease);
    QObject::connect(ui->actionMore, QAction::triggered,
                     &imageGen, &ImageGen::EmitterCountIncrease);

    QObject::connect(ui->actionWavelengthDecrease, QAction::triggered,
                     &imageGen, &ImageGen::WavelengthDecrease);

    QObject::connect(ui->actionWavelengthIncrease, QAction::triggered,
                     &imageGen, &ImageGen::WavelengthIncrease);


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
    ColourMapEditorWidget* colourMapEditor = new ColourMapEditorWidget();
    layoutCentral->addWidget(colourMapEditor);

    // Note: the 'triggered' signal is NOT sent when the value is changed with 'setChecked'.
    //       So, I use 'toggled' instead
    QObject::connect(ui->actionMaskEdit, QAction::toggled,
                     &imageGen, &ImageGen::OnMaskEditChange);

    QObject::connect(ui->actionShowMaskChart, QAction::toggled,
                     colourMapEditor, &ColourMapEditorWidget::SetMaskChartVisible);

    // actionMaskEnable is handled by the funtion on_actionMaskEnable_triggered

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
    case Qt::Key_1:
        imageGen.s.wavelength *= 1.5;
        imageGen.NewImageNeeded();
        qDebug("Image regenerated. Wavelength = %.2f", imageGen.s.wavelength);
        break;
    case Qt::Key_2:
        imageGen.testVal *= 2;
        qDebug("TestVal = %.3f", imageGen.testVal);
        break;
    case Qt::Key_3:
        imageGen.testVal *= 0.5;
        qDebug("TestVal = %.3f", imageGen.testVal);
        break;
    case Qt::Key_4:
        previewScene->ListAllItems();
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
    colourMap.SetMaskEnable(checked);
    if (!checked) {
        ui->actionShowMaskChart->setChecked(false);
    }
    ui->actionMaskEdit->setChecked(checked);
}
