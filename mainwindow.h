#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "datatypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    PreviewView * previewView;
    PreviewScene * previewScene;

private:
    Ui::MainWindow *ui;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);


    // QWidget interface
protected:
private slots:
    void on_actionMore_triggered();
    void on_actionFewer_triggered();
    void on_actionMirrorHor_triggered(bool checked);
    void on_actionMirrorVert_triggered(bool checked);
};
#endif // MAINWINDOW_H
