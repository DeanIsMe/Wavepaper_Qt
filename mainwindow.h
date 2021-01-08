#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QPlainTextEdit>
#include "datatypes.h"
#include "interact.h"

class ColourMapEditorWidget;

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
    QPlainTextEdit * textWindow;
    ColourMapEditorWidget* colourMapEditor;


    void OnInteractChange(QVariant interactType);
private:
    Ui::MainWindow *ui;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);


    // QWidget interface
protected:
private slots:
    void on_actionMirrorHor_triggered(bool checked);
    void on_actionMirrorVert_triggered(bool checked);
    void on_actionMaskEnable_triggered(bool checked);
    void on_actionHideEmitters_toggled(bool arg1);
    void on_actionMaskEdit_toggled(bool arg1);
    void on_actionColoursEdit_toggled(bool arg1);
    void on_actionEditGroup_toggled(bool arg1);
};
#endif // MAINWINDOW_H
