#include "interact.h"
#include "imagegen.h"

#include <QMouseEvent>

Interact_C interact;

void Interact_C::Cancel()
{
    // Restore the backup
    if (active) {
        if (grpActive) {
            *grpActive = grpBackup;
        }
    }
    active = false;
}

Interact_C::Interact_C() {

}

void Interact_C::mousePressEvent(QMouseEvent *event)
{
    Cancel();
    grpActive = imageGen.GetActiveArrangement();
    if (!grpActive) {
        // No active groups
        return;
    }
    active = true;
    grpBackup = *grpActive; // Save, so that it can be reverted
    pressPos = event->localPos();
}

void Interact_C::mouseReleaseEvent(QMouseEvent *event)
{
    (void) event;
    active = false;
}

void Interact_C::mouseMoveEvent(QMouseEvent *event)
{
    if (!active) {return;}
    // Determine how much the mouse has moved while clicked
    qreal moveDistY = pressPos.y() - event->localPos().y();
    grpActive->arcRadius = std::max(0.0, grpBackup.arcRadius + moveDistY);
}
