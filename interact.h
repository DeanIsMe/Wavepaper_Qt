#ifndef INTERACT_H
#define INTERACT_H

#include <QWidget>
#include "imagegen.h"

class Interact_C
{
private:
    bool active = false;
    EmArrangement grpBackup;
    EmArrangement * grpActive;
    QPointF pressPos;
public:

private:
    void Cancel();
public:
    Interact_C();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

extern Interact_C interact;

#endif // INTERACT_H
