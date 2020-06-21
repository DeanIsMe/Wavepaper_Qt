#ifndef PREVIEW_H
#define PREVIEW_H

#include <QWidget>
#include "datatypes.h"

class Preview : public QWidget
{
    Q_OBJECT
public:
    explicit Preview(QWidget *parent = nullptr);


signals:


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // PREVIEW_H
