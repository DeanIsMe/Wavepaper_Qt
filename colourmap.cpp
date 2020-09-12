#include "colourmap.h"
#include "imagegen.h"
#include <QLayout>
#include <QPixmap>
#include <QPlainTextEdit>

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
    const qreal fb = (loc - locBefore);
    const qreal fa = 1. - fb;

    QColor& before = clrIndexed[locBefore];
    QColor& after = clrIndexed[locBefore + 1];
    return qRgb(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
             fa * before.blueF() + fb * after.blueF());
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
    QColor ret;
    ret.setRgbF(f2 * before.clr.red() + f * after.clr.red(),
                f2 * before.clr.green() + f * after.clr.green(),
                f2 * before.clr.blue() + f * after.clr.blue());
    return ret;
}

QRgb ColourMap::RgbInterpolate(qreal loc, const ClrFix &before, const ClrFix &after) {
    qreal f = (loc - before.loc) / (after.loc - before.loc);
    qreal f2 = 1 - f;
    return qRgb(f2 * before.clr.red() + f * after.clr.red(),
                  f2 * before.clr.green() + f * after.clr.green(),
                  f2 * before.clr.blue() + f * after.clr.blue());
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/


ClrFixModel::ClrFixModel(ColourMap * clrMapIn) : clrMap(clrMapIn)
{

}

int ClrFixModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return clrMap->clrList.length();
}

int ClrFixModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ClrFixModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= clrMap->clrList.length()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 1) {
            return QString::asprintf("0x%06X", clrMap->clrList[index.row()].clr.value());
        }
        else if (index.column() == 2) {
            return QString::asprintf("%.1f", clrMap->clrList[index.row()].loc);
        }
        break;
    case Qt::BackgroundRole:
        if (index.column() == 0) {
            return QBrush(clrMap->clrList[index.row()].clr);
        }
        break;
    }

    return QVariant();
}

/** ****************************************************************************
 * @brief ColourMapWidget::ColourMapWidget
 * @param parent
 */
ColourMapWidget::ColourMapWidget(QWidget *parent) : modelClrFix(&colourMap)
{
    Q_UNUSED(parent);
    // Create the widget
    // Sliders, then colour map display
    QVBoxLayout clrMapLayout;
    this->setLayout(&clrMapLayout);

    // Colour bar
    QSize sizeClrBar(200, 20);
    Rgb2D_C* dataClrBar = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    for (int x = dataClrBar->xLeft; x < dataClrBar->xLeft + dataClrBar->width; x++) {
        QRgb clr = colourMap.GetColourValue(100. * (qreal)x / (qreal)sizeClrBar.width());
        for (int y = dataClrBar->yTop; y < dataClrBar->yTop + dataClrBar->height; y++) {
            dataClrBar->setPoint(x, y,  clr);
        }
    }
    imgClrBar = QImage((uchar*)dataClrBar->getDataPtr(), sizeClrBar.width(), sizeClrBar.height(), QImage::Format_ARGB32,
                       ImageDataDealloc, dataClrBar);

    lblClrBar.setPixmap(QPixmap::fromImage(imgClrBar));
    clrMapLayout.addWidget(&lblClrBar);

    // Table
    tableClrFix.setModel(&modelClrFix);
    clrMapLayout.addWidget(&tableClrFix);

    this->show();
}

ColourMapWidget::~ColourMapWidget()
{

}

