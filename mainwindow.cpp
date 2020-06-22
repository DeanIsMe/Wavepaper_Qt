#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include "previewscene.h"
#include "imagegen.h"

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

    PreviewScene * previewScene = new PreviewScene;

    imageGen.AddEmitters(previewScene);
    previewView->setScene(previewScene);

    QImage image;
    imageGen.GenerateImage(image);
    previewScene->addPixmap(QPixmap::fromImage(image, Qt::AutoColor));


    //imageGen.DrawPreview(previewView);

}

MainWindow::~MainWindow()
{
    delete ui;
}

