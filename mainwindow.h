// Wavepaper
// Dean Reading
// 2021

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QPlainTextEdit>
#include <QLayout>
#include "datatypes.h"
#include "interact.h"
#include "valueEditors.h"
#include "colourmap.h"

class ColourMapEditorWidget;

#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define IS_RELEASE 0

// Qt Charts can be difficult, so here's the option to avoid it. Can turn this off for ease of compiling. Will also need to comment the line "QT += charts" in WavePaper.pro
#define USE_QT_CHARTS 1 // 0/1. Should normally be 1.

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitMode();

    PreviewView * previewView = nullptr;
    PreviewScene * previewScene = nullptr;
    QPlainTextEdit * textWindow = nullptr;
    ColourMapEditorWidget * colourMapEditor = nullptr;
    QWidget * editorColDummyWidget = nullptr;
    ValueEditorGroupWidget * valueEditorWidget = nullptr;
    ValueEditorGroupWidget * emitterValEditor = nullptr;
    ValueEditorGroupWidget * imgSizeValEditor = nullptr;
    QScrollArea * valueEditorScroll = nullptr;
    Interact interact;
    ProgramMode programMode;
    QString versionString;

public slots:
    void OnInteractChange(QVariant interactType);
    void ChangeModeToWaves();
    void ChangeModeToFourBar();
private:
    Ui::MainWindow *ui;
    QWidget centralWidget;
    QHBoxLayout layoutCentral;


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
    void on_actionFbEditLengths_toggled(bool arg1);
    void on_actionFbEditAngleInc_toggled(bool arg1);
    void on_actionFbEditDrawRange_toggled(bool arg1);
    void on_actionImageSize_triggered(bool checked);
    void on_actionEditGroup2_toggled(bool arg1);
    void on_actionWavelengthEdit_toggled(bool arg1);
    void on_actionEditWavelength_toggled(bool arg1);
    void on_actionEditLocation_toggled(bool arg1);
};
#endif // MAINWINDOW_H
