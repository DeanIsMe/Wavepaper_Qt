#ifndef COLOURMAP_H
#define COLOURMAP_H

#include "datatypes.h"
#include <QList>
#include <QColor>
#include <QWidget>
#include <QImage>
#include <QAbstractTableModel>
#include <QLabel>
#include <QTableView>

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

/** ****************************************************************************
 * @brief The ColourMap class performs that actual mapping from value to colour
 * for generating the images and previews.
 */
class ColourMap
{
    friend class ClrFixModel;
    // ColourMap class performs that actual mapping from value to colour
    // for generating the images and previews
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


/** ****************************************************************************
 * @brief The ClrFixModel class is a model (see Qt Model/View structures)
 * that tells the view what to display
 */
class ClrFixModel : public QAbstractTableModel
{
    Q_OBJECT;
public:
    ClrFixModel(ColourMap *clrMapIn);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
    ColourMap * clrMap;
};


/** ****************************************************************************
 * @brief The ColourMapWidget class creates a user interface to allow the user
 * to customise a colour map.
 */
class ColourMapWidget : public QWidget {
public:
    ColourMapWidget(QWidget *parent = nullptr);
    ~ColourMapWidget();

private:
    QImage imgClrBar; // Image for the colour bar that represents the map
    ClrFixModel modelClrFix;
    QLabel lblClrBar;
    QTableView tableClrFix;
};

extern ColourMap colourMap;

#endif // COLOURMAP_H
