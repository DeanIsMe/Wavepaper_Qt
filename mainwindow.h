#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
};
#endif // MAINWINDOW_H
