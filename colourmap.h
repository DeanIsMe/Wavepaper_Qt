#ifndef COLOURMAP_H
#define COLOURMAP_H

#include "datatypes.h"
#include <QList>
#include <QColor>


struct ClrFix {
    QColor clr; // Colour at this location
    qreal loc; // 0 to 100
    ClrFix(QColor _clr, qreal _loc) : clr(_clr), loc(_loc) {}
    bool operator <(const ClrFix& other) const {
        return loc < other.loc;
    }
    bool operator >(const ClrFix& other) const {
        return loc > other.loc;
    }
    bool operator==(const ClrFix& other) const {
        return loc == other.loc;
    }
};

class ColourMap
{
public:
    ColourMap();
    void AddColour(QColor clr, qreal loc);
    void AddColour(ClrFix clrFix);

    QRgb GetColourValue(qreal loc);


    void CreateIndexed();
protected:
    QList<ClrFix> clrList;
    QVector<QColor> clrIndexed; // All colours from 0 to 100. !@#$ delete
    static QColor Interpolate(qreal loc, const ClrFix& before, const ClrFix& after);
    static QRgb RgbInterpolate(qreal loc, const ClrFix &before, const ClrFix &after);
};

extern ColourMap colourMap;

#endif // COLOURMAP_H
