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

/** ****************************************************************************
 * @brief The ClrFix struct
 */
struct ClrFix {
    QColor clr; // Colour at this location
    qreal loc; // 0.0 to 1.0
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
    struct MaskCfg { // Mask settings
        qreal numRevs = 3; // How many ripples from from min to max
        qreal offset = 0; // Phase offset, as a count of periods. Should be 0 to 1.
        qreal dutyCycle = 0.3; // 0.5 for even (50%). Must be 0 to 1.0
        qreal widthFactor = 0.5; // Width of transition. 0=immediate transition. 1.0=transition is 50% of period.
        QColor backColour = QColor(0,0,0); // When mask is 0%, what the colour will be
    };
private:
    MaskCfg m;
public:

    ColourMap();

    QRgb GetColourValue(qreal loc) const;
    QRgb GetBlendedColourValue(qreal loc) const;
    QRgb GetBaseColourValue(qreal loc) const;
    qreal GetMaskValue(qreal loc) const;

    ClrFix GetClrFix(qint32 index) const;
    qint32 GetColourFixCount() const {return clrList.length();}

    void AddColour(QColor clr, qreal loc, qint32 listIdx = -1);
    void AddColour(ClrFix clrFix, qint32 listIdx = -1);
    bool RemoveColour(qint32 listIdx);
    void MoveColour(qint32 listIdxFrom, qint32 listIdxTo) {clrList.move(listIdxFrom, listIdxTo);}
    void EditColourLoc(qint32 listIdx, qreal newLoc);
    void EditColour(qint32 listIdx, QRgb newClr);
    void CalcColourIndex();
    const MaskCfg& GetMaskConfig() {return m;}
    void SetMaskConfig(MaskCfg& maskCfgIn) {m = maskCfgIn; MaskChanged();}
    bool RecalcPending() {return pendingRecalcClrIndex || pendingRecalcMaskIndex;}

private:
    void CalcMaskIndex();

public:
    static const int clrIndexMax = 200; // The colour indices span from 0 to this number
protected:
    QList<ClrFix> clrList;
    QVector<QColor> clrIndexed; // All colours from locations 0 to 1.0 (indices 0 to clrIndexMax)
    QVector<qreal> maskIndexed; // All mask values from locations 0 to 1.0 (indices 0 to clrIndexMax). Values are 0 to 1.0.
    bool pendingRecalcClrIndex = true;
    bool pendingRecalcMaskIndex = true;

    static QColor Interpolate(qreal loc, const ClrFix& before, const ClrFix& after);
    static QRgb RgbInterpolate(qreal loc, const ClrFix &before, const ClrFix &after);

    void ClrListChanged(); // Called any time an edit is made to the colour list
    void MaskChanged(); // Called any time an edit is made to the mask

private slots:
    void RecalcSlot();

signals:
    void ClrMapChanged(); // Emitted any time an edit is made to the mask
    void NewClrMapReady(); // Emitted after the new colour index is made - ready to be used by other parts of the program
};


/** ****************************************************************************
 * @brief The ClrFixModel class is a model (see Qt Model/View structures)
 * that tells the view what to display
 */
class ClrFixModel : public QAbstractTableModel
{
    friend class ColourMapEditorWidget;
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


    // QAbstractItemModel interface
public:
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int startRow, int rowCount, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

/** ****************************************************************************
 * @brief The ClrFixModel class is a model (see Qt Model/View structures)
 * that tells the view what to display
 */
class ClrFixTableView : public QTableView {
    Q_OBJECT
public:
    ClrFixTableView() {}

    void RemoveSelectedRows();
    void AddRow();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};


/** ****************************************************************************
 * @brief The ColourMapEditorWidget class creates a user interface to allow the user
 * to customise a colour map.
 */
class ColourMapEditorWidget : public QWidget {
    Q_OBJECT
private:
    static constexpr int heightClrBar = 30;
public:
    ColourMapEditorWidget(QWidget *parent = nullptr);
    ~ColourMapEditorWidget();

private slots:
    void DrawColourBars();

private:
    QImage imgBarBase; // Image for the colour bar that represents the base colour map
    QImage imgBarMask; // Image for the colour bar that represents the colour map mask
    QImage imgBarResult; // Image for the colour bar that represents the resultant colour map
    ClrFixModel modelClrFix;
    QLabel lblClrBarBase;
    QLabel lblClrBarMask;
    QLabel lblClrBarResult;

    ClrFixTableView tableClrFix;
};

extern ColourMap colourMap;

#endif // COLOURMAP_H
