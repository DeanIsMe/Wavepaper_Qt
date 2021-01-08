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
#include <QChart>
#include <QChartView>
#include <QLineSeries>
using namespace QtCharts;


/** ****************************************************************************
 * @brief The ColourMap class performs that actual mapping from value to colour
 * for generating the images and previews.
 * It also implements QAbstractTableModel to interface with the colour map table
 */
class ColourMap : public QObject
{
    Q_OBJECT
    friend class ColourMapEditorWidget;
    // ColourMap class performs that actual mapping from value to colour
    // for generating the images and previews
public:
    ColourMap(ColourList & clrListIn, MaskCfg & maskCfgIn, ImageGen& imgGenIn);

    QRgb GetColourValue(const GenSettings &genSet, qreal loc) const;
    QRgb GetBaseColourValue(const GenSettings &genSet, qreal loc) const;
    qreal GetMaskValue(const GenSettings &genSet, qreal loc) const;
    QRgb GetColourValueIndexed(const GenSettings &genSet, qreal loc) const;
    QRgb GetBlendedColourValue(const GenSettings &genSet, qreal loc) const;

    static ClrFix GetClrFix(ColourList &colourList, qint32 index);
    qint32 GetColourFixCount() const {return clrList.length();}

    void AddColour(QColor clr, qreal loc, qint32 listIdx = -1);
    void AddColour(ClrFix clrFix, qint32 listIdx = -1);
    static void EditColourLoc(ColourList &colourList, qint32 listIdx, qreal newLoc);
    void EditColour(qint32 listIdx, QRgb newClr);

    void SetColourList(ColourList& clrListIn);

    void SetPreset(ClrMapPreset preset);

    void CalcColourIndex(GenSettings &genSet);
    void CalcMaskIndex(GenSettings &genSet);

signals:
    PreColourListReset();
    PostColourListReset();

public slots:
    void SetPresetHot() {SetPreset(ClrMapPreset::hot);}
    void SetPresetCool() {SetPreset(ClrMapPreset::cool);}
    void SetPresetJet() {SetPreset(ClrMapPreset::jet);}
    void SetPresetBone() {SetPreset(ClrMapPreset::bone);}
    void SetPresetParula() {SetPreset(ClrMapPreset::parula);}
    void SetPresetHsv() {SetPreset(ClrMapPreset::hsv);}


protected:
    ColourList & clrList; // Used for QAbstractTableModel
    MaskCfg & maskCfg;
    ImageGen & imgGen;

protected:
    static QColor Interpolate(qreal loc, const ClrFix& before, const ClrFix& after);
    static QRgb RgbInterpolate(qreal loc, const ClrFix &before, const ClrFix &after);

    void ClrListChanged(); // Called any time an edit is made to the colour list
    void MaskSettingChanged(); // Called any time an edit is made to the mask
};

/** ****************************************************************************
 * @brief The ClrListTableModel class
 */
class ClrListTableModel : public QAbstractTableModel {
    Q_OBJECT
    friend class ClrListTableView;
    friend class ColourMapEditorWidget;
protected:
    // Column numbers
    static constexpr int colClrBox = 0;
    static constexpr int colClrHex = 1;
    static constexpr int colLoc = 2;
    static constexpr int colCount = 3;
public:
    ClrListTableModel(ImageGen& imgGenIn);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
public slots:
    void TableClicked(const QModelIndex & index);
    void PreColourListResetSlot() {this->beginResetModel();}
    void PostColourListResetSlot() {this->endResetModel();}

private:
    ImageGen& imgGen;
    ColourList & clrList;
    // QAbstractItemModel interface
public:
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int startRow, int rowCount, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

/** ****************************************************************************
 * @brief The ClrListTableView class displays the colour map in a table
 */
class ClrListTableView : public QTableView {
    Q_OBJECT
public:
    ClrListTableView() {}

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
    ColourMapEditorWidget(ImageGen& imgGenIn);
    ~ColourMapEditorWidget();
    void DrawColourBars(GenSettings &genSet);

public slots:
    void SetMaskChartVisible(bool on);

private:
    ImageGen& imgGen;
    ColourMap * clrMap;

    qint32 barWidth; // The width of the colour bar image
    QImage imgBarBase; // Image for the colour bar that represents the base colour map
    QImage imgBarMask; // Image for the colour bar that represents the colour map mask
    QImage imgBarResult; // Image for the colour bar that represents the resultant colour map

    // Labels are used as a colour bar to demonstrate the colour map
    QLabel lblClrBarBase;
    QLabel lblClrBarMask;
    QLabel lblClrBarResult;

    ClrListTableModel clrListModel;
    ClrListTableView clrListTable; // Table for editing the colours

    // Line chart to display the mask
    QLineSeries * maskSeries = nullptr;
    QChart maskChart;
    QChartView maskChartView;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // COLOURMAP_H
