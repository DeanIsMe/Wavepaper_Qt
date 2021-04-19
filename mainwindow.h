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
    EditorGroupWidget * valueEditorWidget = nullptr;
    Interact interact;
    ProgramMode programMode;

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
};
#endif // MAINWINDOW_H
