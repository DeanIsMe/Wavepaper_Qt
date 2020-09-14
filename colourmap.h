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
class ColourMap : public QObject
{
    Q_OBJECT;
    // ColourMap class performs that actual mapping from value to colour
    // for generating the images and previews
public:
    ColourMap();

    QRgb GetColourValue(qreal loc) const;

    ClrFix GetClrFix(qint32 index) const;
    qint32 GetColourFixCount() const {return clrList.length();}

    void AddColour(QColor clr, qreal loc);
    void AddColour(ClrFix clrFix);
    void EditColourLoc(qint32 index, qreal newLoc);
    void EditColour(qint32 index, QRgb newClr);
    void CreateIndex();

protected:
    QList<ClrFix> clrList;
    QVector<QColor> clrIndexed; // All colours from 0 to 100. !@#$ delete
    static QColor Interpolate(qreal loc, const ClrFix& before, const ClrFix& after);
    static QRgb RgbInterpolate(qreal loc, const ClrFix &before, const ClrFix &after);
signals:
    void ClrListChanged(); // Emitted automatically any time an edit is made
    void NewClrMapReady(); // Emitted after the new colour index is made - ready to be used by other parts of the program
};


/** ****************************************************************************
 * @brief The ClrFixModel class is a model (see Qt Model/View structures)
 * that tells the view what to display
 */
class ClrFixModel : public QAbstractTableModel
{
    friend class ColourMapWidget;
    Q_OBJECT;
    static constexpr int colClrBox = 0;
    static constexpr int colClrHex = 1;
    static constexpr int colLoc = 2;
public:
    ClrFixModel(ColourMap *clrMapIn);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
public slots:
    void TableClicked(const QModelIndex & index);
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
