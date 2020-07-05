#include "mainwindow.h"
#include "datatypes.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}


/** ****************************************************************************
 * @brief Snap snaps a value to certain increments with a
 * given strength. Utility function.
 * @param val
 * @param snapInc defines the locations to snap to
 * @param snapWithin defines the strength of the snapping.
 * If val is within 'snapWithin' of a snap location, then
 * snapping will occur. Otherwise, no snapping.
 * If snapWithin isn't supplied, then we'll always snap.
 * @return
 */
qreal Snap(qreal val, qreal snapInc, qreal snapWithin) {
    qint32 div = qRound(val / snapInc);
    qreal rem = val - div * snapInc;
    if (fabs(rem) <= snapWithin) {
        // Snap!
        return div * snapInc;
    }
    // Don't snap
    return val;
}

qreal Snap(qreal val, qreal snapInc) {
    // Always snap
    return qRound(val / snapInc) * snapInc;
}

qint32 Snap(qint32 val, qint32 snapInc, qint32 snapWithin) {
    qint32 distFromSnap = modPos(val + snapInc/2, snapInc) - snapInc/2;
    if (distFromSnap <= snapWithin && distFromSnap >= -snapWithin) {
        // Snap the value!
        return val - distFromSnap;
    }
    // Don't snap
    return val;
}
