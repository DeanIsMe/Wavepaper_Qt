#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include <QBrush>
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    PreviewView * previewView = new PreviewView(this);
    layoutCentral->addWidget(previewView);

    PreviewScene * previewScene = new PreviewScene(this);
    previewScene->setSceneRect(imageGen.simArea);
    previewView->setScene(previewScene);

    imageGen.AddEmitters(previewScene);

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


    // Print test image
    TestWidget * testCanvas = new TestWidget(this);
    testCanvas->setMinimumSize(200, 300);
    layoutCentral->addWidget(testCanvas);
}

MainWindow::~MainWindow()
{
    delete ui;
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
