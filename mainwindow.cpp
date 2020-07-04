#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include <QBrush>
#include <QKeyEvent>
#include "previewscene.h"
#include "imagegen.h"

class TestWidget : public QWidget {
    QImage image;
public:
    explicit TestWidget(QWidget * parent) : QWidget(parent) {
        imageGen.GenerateImage(image);
    }
protected:
    void paintEvent(QPaintEvent *event);
};

/** ****************************************************************************
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    imageGen.s.emitterRadius = 2.0;

    QWidget * central = new QWidget;
    setCentralWidget(central);
    QHBoxLayout * layoutCentral = new QHBoxLayout;
    central->setLayout(layoutCentral);

    // Example button column
    QVBoxLayout * layoutButtonCol = new QVBoxLayout;

    QPushButton * btnGo = new QPushButton("Go");
    QPushButton * btnTwo = new QPushButton("Two");
    layoutButtonCol->addWidget(btnGo);
    layoutButtonCol->addWidget(btnTwo);

    layoutCentral->addLayout(layoutButtonCol);

    imageGen.InitViewAreas();

    previewView = new PreviewView(this);
    layoutCentral->addWidget(previewView);

    previewScene = new PreviewScene(previewView);
    previewScene->setSceneRect(imageGen.simArea);
    previewView->setScene(previewScene);

    previewView->setSceneRect(imageGen.simArea);

    QObject::connect(&imageGen, &ImageGen::PreviewImageChanged,
                     previewView, &PreviewView::OnBackgroundChange);

    QObject::connect(&imageGen, &ImageGen::EmittersChanged,
                     previewScene, &PreviewScene::OnEmitterChange);

    previewScene->AddEmitters(imageGen);

    imageGen.GenerateImage(imageGen.image); // Stored for use later

    qDebug() << "Preview view rect " << RectToQString(previewView->rect());
    qDebug() << "Preview view frameRect " << RectToQString(previewView->frameRect());
    qDebug() << "Preview view sceneRect " << RectFToQString(previewView->sceneRect());

    // Generate the image
    // Use background brush
//    QImage * image = new QImage;
//    imageGen.GenerateImage(*image);
//    QBrush brush(*image);
//    previewScene->setBackgroundBrush(brush);

    previewScene->AddEmitters(imageGen);
    emit imageGen.PreviewImageChanged();

    // Print test image
    TestWidget * testCanvas = new TestWidget(this);
    testCanvas->setMinimumSize(200, 300);
    layoutCentral->addWidget(testCanvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Key pressed: " << event->key();
    switch (event->key()) {
    case Qt::Key_1:
        imageGen.s.wavelength *= 1.5;
        imageGen.GenerateImage(imageGen.image);
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
    default:
        event->ignore();
    }

    // !@#$
    imageGen.setTargetImgPoints(100000 * imageGen.testVal);
    imageGen.GenerateImage(imageGen.image);
    previewScene->invalidate(previewView->sceneRect());
    // !@#$
}


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


void TestWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    //PaintTestImage(this);

    QPainter painter(this);
    painter.setWindow(image.rect());
    painter.drawImage(image.rect().x(), image.rect().y(), image);
}

void MainWindow::on_actionMore_triggered()
{
    imageGen.EmitterCountIncrease();
}

void MainWindow::on_actionFewer_triggered()
{
    imageGen.EmitterCountDecrease();
}
