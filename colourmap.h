#ifndef COLOURMAP_H
#define COLOURMAP_H

#include "datatypes.h"
#include <QList>
#include <QColor>

struct ClrFix {
    QColor clr; // Colour at this location
    qreal loc; // 0 to 100
    ClrFix(QColor _clr, qreal _loc) : clr(_clr), loc(_loc) {}
    bool operator <(const ClrFix& other) {
        return loc < other.loc;
    }
    bool operator >(const ClrFix& other) {
        return loc > other.loc;
    }
    bool operator==(const ClrFix& other) {
        return loc == other.loc;
    }
};

bool operator <(const ClrFix& x, const ClrFix& y) {
    return x.loc < y.loc;
}
bool operator==(const ClrFix& x, const ClrFix& y) {
    return x.loc == y.loc;
}

class ColourMap
{
public:
    ColourMap();
    void AddColour(QColor clr, qreal loc);
    void AddColour(ClrFix clrFix);

protected:
    void CreateIndexed();
    QList<ClrFix> clrList;
    QVector<QColor> clrIndexed; // All colours from 0 to 100
    static QColor Interpolate(qreal loc, const ClrFix& before, const ClrFix& after);
};

#endif // COLOURMAP_H
