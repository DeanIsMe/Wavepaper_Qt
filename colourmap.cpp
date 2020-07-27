#include "colourmap.h"

ColourMap colourMap;

ColourMap::ColourMap()
{
    // !@# temporary data
    clrList.reserve(5);
    clrList.append(ClrFix(Qt::green,   0));
    clrList.append(ClrFix(Qt::black,  25));
    clrList.append(ClrFix(Qt::red,  50));
    clrList.append(ClrFix(Qt::black,  75));
    clrList.append(ClrFix(Qt::blue, 100));
    CreateIndexed();
}

void ColourMap::AddColour(ClrFix clrFix)
{
    clrList.append(clrFix);
}

// loc is a number from 0 (min) to 100 (max)
// Retrieves the colour, using the index and interpolation
QRgb ColourMap::GetColourValue(qreal loc) {
    int locBefore = std::min(99, std::max(0,(int)loc));
    qreal f = (loc - locBefore);
    qreal f2 = 1. - f;
    QColor& before = clrIndexed[locBefore];
    QColor& after = clrIndexed[locBefore + 1];
    return qRgb(f2 * before.red() + f * after.red(),
             f2 * before.green() + f * after.green(),
             f2 * before.blue() + f * after.blue());
}

void ColourMap::AddColour(QColor clr, qreal loc)
{
    AddColour(ClrFix(clr, loc));
}

void ColourMap::CreateIndexed()
{
    if (clrList.size() < 2) {
        qFatal("ColourMap::CreateIndexed() not enough colours in list!");
    }
    std::sort(clrList.begin(), clrList.end());

    clrList.first().loc;
    clrIndexed.clear();
    clrIndexed.resize(101);
    for (qint32 i = 0; i < clrIndexed.size(); i++) {
        // Find the colours on either side
        ClrFix before(Qt::transparent, 0), after(Qt::transparent, 100);
        ClrFix thisClr(Qt::transparent, i);
        // Find the first element after this location
        auto afterIt = std::upper_bound(clrList.begin(), clrList.end(), thisClr);
        if (afterIt != clrList.end()) {
            after = *afterIt;
        }
        // Grab the one earlier
        auto beforeIt = afterIt - 1;
        if (beforeIt >= clrList.begin()) {
            before = *beforeIt;
        }
        // Interpolate to find this point (from before and after)
        clrIndexed[i] = Interpolate(i, before, after);
    }
}

QColor ColourMap::Interpolate(qreal loc, const ClrFix &before, const ClrFix &after) {
    qreal f = (loc - before.loc) / (after.loc - before.loc);
    qreal f2 = 1 - f;
    return QColor(f2 * before.clr.red() + f * after.clr.red(),
                  f2 * before.clr.green() + f * after.clr.green(),
                  f2 * before.clr.blue() + f * after.clr.blue());
}
