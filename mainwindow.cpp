#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QWidget>
#include <QPushButton>
#include "preview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Preview * winPreview = new Preview;

    QWidget * winEmitter = new QWidget;
    QVBoxLayout * layoutVert = new QVBoxLayout;


    QPushButton * btnGo = new QPushButton("Go");
    QPushButton * btnTwo = new QPushButton("Two");
    layoutVert->addWidget(btnGo);
    layoutVert->addWidget(btnTwo);

    winEmitter->setLayout(layoutVert);

    QHBoxLayout * layoutHor = new QHBoxLayout;
    layoutHor->addWidget(winEmitter);
    layoutHor->addWidget(winPreview);

    QWidget * central = new QWidget;
    central->setLayout(layoutHor);

    setCentralWidget(central);
}

MainWindow::~MainWindow()
{
    delete ui;
}

