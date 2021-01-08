#include "colourmap.h"
#include "imagegen.h"
#include <QLayout>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QColorDialog>
#include <QHeaderView>
#include <QPushButton>

/** ****************************************************************************
 * @brief ColourMap::ColourMap
 * @param clrListIn
 * @param maskCfgIn
 */
ColourMap::ColourMap(ColourList &clrListIn, MaskCfg &maskCfgIn, ImageGen& imgGenIn) :
    clrList(clrListIn), maskCfg(maskCfgIn), imgGen(imgGenIn)
{

}

/** ****************************************************************************
 * @brief ColourMap::EditColourLoc
 * @param colourList
 * @param listIdx
 * @param newLoc
 */
void ColourMap::EditColourLoc(ColourList& colourList, qint32 listIdx, qreal newLoc)
{
    if (listIdx > colourList.length()) { return; }
    (colourList)[listIdx].loc = qBound(0., newLoc, 1.0);
}

void ColourMap::SetColourList(ColourList &clrListIn) {
    // !@#$ CHANGE THIS
    emit PreColourListReset();
    clrList = clrListIn;
    emit PostColourListReset();
}

/** ****************************************************************************
 * @brief ColourMap::SetPreset replaces the entire colour list with a preset list
 * @param preset
 */
void ColourMap::SetPreset(ClrMapPreset preset)
{
    emit PreColourListReset();

    // !@# enter the correct colour maps
    clrList.clear();
    switch (preset) {
    //clrList.append(ClrFix(QColor(), /64.));

    case ClrMapPreset::hot:
        clrList.append(ClrFix(QColor(10,0,0), 0));
        clrList.append(ClrFix(QColor(255,0,0), 23./63.));
        clrList.append(ClrFix(QColor(255,255,0), 47./63.));
        clrList.append(ClrFix(QColor(255,255,255), 1.0));
        break;
    case ClrMapPreset::cool:
        clrList.append(ClrFix(QColor(0,255,255), 0));
        clrList.append(ClrFix(QColor(255,0,255), 1.0));
        break;
    case ClrMapPreset::jet:
        clrList.append(ClrFix(QColor(0,0,143), 0.));
        clrList.append(ClrFix(QColor(0,0,255), 7./63.));
        clrList.append(ClrFix(QColor(0,255,255), 23./63.));
        clrList.append(ClrFix(QColor(255,255,0), 39./63.));
        clrList.append(ClrFix(QColor(255,0,0), 55./63.));
        clrList.append(ClrFix(QColor(128,0,0), 63./63.));
        break;
    case ClrMapPreset::bone:
        clrList.append(ClrFix(QColor(0,0,1), 0));
        clrList.append(ClrFix(QColor(81,81,113), 23./63.));
        clrList.append(ClrFix(QColor(166,198,198), 47./63.));
        clrList.append(ClrFix(QColor(255,255,255), 1.0));
        break;
    case ClrMapPreset::parula:
        clrList.append(ClrFix(QColor(62,38,168), 0));
        clrList.append(ClrFix(QColor(48,125,251), 66./255.));
        clrList.append(ClrFix(QColor(48,125,251), 66./255.));
        clrList.append(ClrFix(QColor(0,185,199), 119./255.));
        clrList.append(ClrFix(QColor(202,193,40), 192./255.));
        clrList.append(ClrFix(QColor(253,189,61), 214./255.));
        clrList.append(ClrFix(QColor(249,251,20), 1.));
        break;
    case ClrMapPreset::hsv:
        clrList.append(ClrFix(QColor(255,0,0), 0));
        clrList.append(ClrFix(QColor(255,255,0), 1./6.));
        clrList.append(ClrFix(QColor(0,255,0), 2./6.));
        clrList.append(ClrFix(QColor(0,255,255), 3./6.));
        clrList.append(ClrFix(QColor(0,0,255), 4./6.));
        clrList.append(ClrFix(QColor(255,0,255), 5./6.));
        clrList.append(ClrFix(QColor(255,0,0), 6./6.));
        break;
    }
    emit PostColourListReset();
    imgGen.NewImageNeeded();
}

/** ****************************************************************************
 * @brief ColourMap::GetColourValue calculates the colour, using the location and interpolation
 * @param loc is a number from 0 (min) to 1 (max)
 * @return a QRgb, using the alpha layer
 */
QRgb ColourMap::GetColourValue(const GenSettings& genSet, qreal loc) const {
    qreal locAsIndex = (loc * (qreal)genSet.clrIndexMax);
    int idxBefore = std::min(genSet.clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore) * 255.;
    const qreal fa = (255. - fb);
    // fa and fb are multiplied by 255 because qRgba expects values in the range
    // 0 to 255, but redF and such return values in the range 0 to 1.0.

    const QColor& before = genSet.clrIndexed[idxBefore];
    const QColor& after = genSet.clrIndexed[idxBefore + 1];
    return qRgba(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
                fa * before.blueF() + fb * after.blueF(),
                 fa * genSet.maskIndexed[idxBefore] + fb * genSet.maskIndexed[idxBefore + 1]);
}

/** ****************************************************************************
 * @brief ColourMap::GetColourValueIndexed picks the closest index for this colour
 * @param loc is a number from 0 (min) to 1 (max)
 * @return a QRgb, using the alpha layer
 */
QRgb ColourMap::GetColourValueIndexed(const GenSettings& genSet, qreal loc) const {
    int locAsIndex = (loc * (qreal)genSet.clrIndexMax) + 0.5;
    int idx = std::min(genSet.clrIndexMax-1, std::max(0, locAsIndex));

    return (genSet.clrIndexedRgb[idx] & 0xFFFFFF) | (genSet.maskIndexedInt[idx]);
}

/** ****************************************************************************
 * @brief ColourMap::GetBlendedColourValue calculates the colour, using the location and interpolation
 * @param loc is a number from 0 (min) to 1 (max)
 * @returns a QRgb with alpha layer off - the background is pre-blended into the colour
 */
inline QRgb ColourMap::GetBlendedColourValue(const GenSettings& genSet, qreal loc) const {
    qreal locAsIndex = (loc * (qreal)genSet.clrIndexMax);
    int idxBefore = std::min(genSet.clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;

    const QColor& before = genSet.clrIndexed[idxBefore];
    const QColor& after = genSet.clrIndexed[idxBefore + 1];

    qreal mask = (fa * genSet.maskIndexed[idxBefore] + fb * genSet.maskIndexed[idxBefore + 1]) * 255.;
    qreal maskInv = (255. - mask);
    return qRgb(mask * (fa * before.redF() + fb * after.redF()) + maskInv * maskCfg.backColour.redF(),
                mask * (fa * before.greenF() + fb * after.greenF()) + maskInv * maskCfg.backColour.greenF(),
                mask * (fa * before.blueF() + fb * after.blueF()) + maskInv * maskCfg.backColour.blueF());
}


/** ****************************************************************************
 * @brief ColourMap::GetBaseColourValue
 * @param loc
 * @return
 */
inline QRgb ColourMap::GetBaseColourValue(const GenSettings& genSet, qreal loc) const
{
    qreal locAsIndex = (loc * (qreal)genSet.clrIndexMax);
    int idxBefore = std::min(genSet.clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore) * 255.;
    const qreal fa = (255. - fb);
    // fa and fb are multiplied by 255 because qRgba expects values in the range
    // 0 to 255, but redF and such return values in the range 0 to 1.0.

    const QColor& before = genSet.clrIndexed[idxBefore];
    const QColor& after = genSet.clrIndexed[idxBefore + 1];
    return qRgb(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
                fa * before.blueF() + fb * after.blueF());
}

/** ****************************************************************************
 * @brief ColourMap::GetMaskValue
 * @param loc
 * @return the mask value in the range 0 to 1
 */
inline qreal ColourMap::GetMaskValue(const GenSettings& genSet, qreal loc) const
{
    if (!maskCfg.enabled) {return 1.0;}
    qreal locAsIndex = (loc * (qreal)genSet.clrIndexMax);
    int idxBefore = std::min(genSet.clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;
    return fa * genSet.maskIndexed[idxBefore] + fb * genSet.maskIndexed[idxBefore + 1];
}

/** ****************************************************************************
 * @brief ColourMap::GetClrFix
 * @param index
 * @return
 */
ClrFix ColourMap::GetClrFix(ColourList& colourList, qint32 index)
{
    if (index > colourList.length()) { return ClrFix(QRgb(), 0); }
    return (colourList)[index];
}

/** ****************************************************************************
 * @brief ColourMap::CalcColourIndex
 */
void ColourMap::CalcColourIndex(GenSettings& genSet)
{

    qDebug("CalcColourIndex");
    if (clrList.length() < 2) {
        qFatal("ColourMap::CreateIndexed() not enough colours in list!");
    }
    QVector<QColor>& clrIndexed = genSet.clrIndexed;
    QVector<QRgb>& clrIndexedRgb = genSet.clrIndexedRgb;

    std::sort(clrList.begin(), clrList.end());

    // Calculate colour index
    clrIndexed.clear();
    clrIndexed.resize(genSet.clrIndexMax + 1);
    for (qint32 i = 0; i < clrIndexed.size(); i++) {
        qreal loc = (qreal)i / genSet.clrIndexMax;
        // Find the colours on either side
        ClrFix before(Qt::transparent, 0), after(Qt::transparent, 1.0);
        ClrFix thisClr(Qt::transparent, loc);
        // Find the first element after this location
        auto afterIt = std::upper_bound(clrList.begin(), clrList.end(), thisClr);
        auto beforeIt = afterIt - 1; // The colour fix before that
        after = afterIt == clrList.end() ? *beforeIt : *afterIt;
        before = beforeIt >= clrList.begin() ? *beforeIt : *afterIt;
        // Interpolate to find this point (from before and after)
        clrIndexed[i] = Interpolate(loc, before, after);
    }

    clrIndexedRgb.clear();
    clrIndexedRgb.resize(genSet.clrIndexMax + 1);
    for (qint32 i = 0; i < clrIndexedRgb.size(); i++) {
        clrIndexedRgb[i] =  clrIndexed[i].rgb();
    }
}

/** ****************************************************************************
 * @brief ColourMap::CalcMaskIndex
 */
void ColourMap::CalcMaskIndex(GenSettings& genSet)
{
    qDebug("CalcMaskIndex. Revs=%.2f, Duty=%.2f, Smooth=%.2f, Offset=%.2f", maskCfg.numRevs, maskCfg.dutyCycle, maskCfg.smooth, maskCfg.offset);

    QVector<qreal>& maskIndexed = genSet.maskIndexed; // All mask values from locations 0 to 1.0 (indices 0 to clrIndexMax). Values are 0 to 1.0.
    QVector<quint32>& maskIndexedInt = genSet.maskIndexedInt; // All mask values from locations 0 to 1.0 (indices 0 to clrIndexMax). Values are (0 to 255) << 24

    qint32 maskLen = genSet.clrIndexMax + 1;
    maskIndexed.clear();
    maskIndexed.resize(maskLen);
    maskIndexedInt.clear();
    maskIndexedInt.resize(maskLen);
    if (!maskCfg.enabled) {
        maskIndexed.fill(1.0);
        maskIndexedInt.fill(0xFF000000);
        return;
    }

    qint32 period = std::max(2, (qint32)(maskLen/maskCfg.numRevs)); // period as #points
    qint32 highLen = qBound(1, (int) (maskCfg.dutyCycle * period), period); // #points in high(1) (including 0 to 1 transition)
    qint32 lowLen = period - highLen; // #points in low(0)

    // Pre-calculate the transitions
    qint32 desTransLen = (int)(maskCfg.smooth*0.5*maskLen/maskCfg.numRevs);
    qint32 transitionLen0 = qBound(1, desTransLen, lowLen);
    qint32 transitionLen1 = qBound(1, desTransLen, highLen);
    QVector<double> transition0(transitionLen0);
    if (transitionLen0 == 1) {transition0[0] = 0;}
    else {
        double delta = 3.1415926 / (double) (transitionLen0 - 1);
        for (qint32 i = 0; i < transition0.size(); i++) {
            transition0[i] = 0.5 + 0.5 * cos(i * delta); // 1 to 0
        }
    }
    QVector<double> transition1(transitionLen1);
    if (transitionLen1 == 1) {transition1[0] = 0;}
    else {
        double delta = 3.1415926 / (double) (transitionLen1 - 1);
        for (qint32 i = 0; i < transition1.size(); i++) {
            transition1[i] = 0.5 - 0.5 * cos(i * delta); // 1 to 0
        }
    }
    // Transition start location (as an index). Should always be <= thisIdx
    // The start location is chosen such that the peaks are at roughly the same location
    // as the transition width/smoothin factor is changed
    qint32 activeTransIdx = maskCfg.offset * (qreal) period - transitionLen1 + (highLen - transitionLen1) / 2;
    while (activeTransIdx > 0) { activeTransIdx -= period; }
    qint8 transVal = 1; // The value that this transition is moving to.  0: high to low.  1: low to high.
    qint32 thisIdx = 0;
    while (thisIdx < maskLen) {
        // Check for an update on the active transition
        if (transVal) {
            if (thisIdx >= activeTransIdx + highLen) {
                activeTransIdx += highLen;
                transVal = 0;
                continue;
            }
        }
        else {
            if (thisIdx >= activeTransIdx + lowLen) {
                activeTransIdx += lowLen;
                transVal = 1;
                continue;
            }
        }

        // Calculate the value for this index
        const qint32 thisTransIdx = thisIdx - activeTransIdx;
        qreal thisVal;

        // Currently in a transition
        if (transVal) { // 0 to 1
            if (thisTransIdx < transitionLen1) {
                thisVal = transition1[thisTransIdx]; // Transitioning
            }
            else {
                thisVal = 1; // Finished transition
            }
        }
        else { // 1 to 0
            if (thisTransIdx < transitionLen0) {
                thisVal = transition0[thisTransIdx]; // Transitioning
            }
            else {
                thisVal = 0; // Finished transition
            }
        }
        maskIndexed[thisIdx] = thisVal;
        maskIndexedInt[thisIdx] = ((quint32)(255. * thisVal)) << 24;
        thisIdx++;
    }
}

/** ****************************************************************************
 * @brief ColourMap::Interpolate
 * @param loc
 * @param before
 * @param after
 * @return
 */
QColor ColourMap::Interpolate(qreal loc, const ClrFix &before, const ClrFix &after) {
    qreal f = (loc - before.loc) / (after.loc - before.loc);
    if (qIsNaN(f)) {f = 1.0;}
    qreal f2 = 1 - f;
    QColor ret;
    ret.setRgbF(f2 * before.clr.redF() + f * after.clr.redF(),
                f2 * before.clr.greenF() + f * after.clr.greenF(),
                f2 * before.clr.blueF() + f * after.clr.blueF());
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
ClrListTableModel::ClrListTableModel(ImageGen &imgGenIn) : imgGen(imgGenIn), clrList(imgGenIn.s.clrList) {}


int ClrListTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return clrList.length();
}

int ClrListTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return colCount;
}

/** ****************************************************************************
 * @brief ClrListTableModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ClrListTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= clrList.length()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: // What is displayed when editing text
        if (index.column() == colClrHex) {
            const QColor& clr = clrList[index.row()].clr;
            return QString::asprintf("%02X %02X %02X", clr.red(), clr.green(), clr.blue());
        }
        else if (index.column() == colLoc) {
            return QString::asprintf("%.1f", clrList[index.row()].loc);
        }
        break;
    case Qt::BackgroundRole:
        if (index.column() == colClrBox) {
            return QBrush(clrList[index.row()].clr);
        }
        break;
    }

    return QVariant();
}

/** ****************************************************************************
 * @brief ClrListTableModel::setData
 * @param index
 * @param value
 * @param role
 * @return
 */
bool ClrListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index)) { return false; }
        if (index.column() == colLoc && index.row() < clrList.length()) {
            clrList[index.row()].loc = value.toReal();
            emit dataChanged(index, index);
            imgGen.NewImageNeeded();
            return true;
        }
    }
    return false;
}

/** ****************************************************************************
 * @brief ClrListTableModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags ClrListTableModel::flags(const QModelIndex &index) const
{
    if (index.column() == colLoc) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
    else if (index.column() == colClrBox) {
        return QAbstractTableModel::flags(index) & ~(Qt::ItemIsSelectable);
    }
    else if (index.column() == colClrHex) {
        return QAbstractTableModel::flags(index) & ~(Qt::ItemIsSelectable);
    }
    return QAbstractTableModel::flags(index);
}

/** ****************************************************************************
 * @brief ClrListTableModel::TableClicked
 * @param index
 */
void ClrListTableModel::TableClicked(const QModelIndex &index)
{
    if (index.row() >= clrList.length()) {
        return;
    }
    if (index.column() == colClrBox) {
        QColor clrNew = QColorDialog::getColor(clrList[index.row()].clr);
        if (!clrNew.isValid()) {return;} // User cancelled
        QRgb clrRgbNew = clrNew.rgb();
        if (index.row() < clrList.length()) {
            clrList[index.row()].clr = clrRgbNew;
            imgGen.NewPreviewImageNeeded();
        }
        emit dataChanged(index, index);
        QModelIndex idxHex = createIndex(index.row(), colClrHex);
        emit dataChanged(idxHex, idxHex);
    }
}

/** ****************************************************************************
 * @brief ClrListTableModel::removeRows
 * @param startRow
 * @param rowCount
 * @param parent
 * @return
 */
bool ClrListTableModel::removeRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        if (startRow < clrList.length()) {
            clrList.removeAt(startRow);
        }
    }
    endRemoveRows();
    imgGen.NewPreviewImageNeeded();
    return true;
}

/** ****************************************************************************
 * @brief ClrListTableModel::insertRows
 */
bool ClrListTableModel::insertRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        clrList.insert(startRow, ClrFix(Qt::white, 1.0));
    }
    endInsertRows();
    imgGen.NewPreviewImageNeeded();
    return true;
}

/** ****************************************************************************
 * @brief ClrListTableModel::moveRows
 */
bool ClrListTableModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    int destRow = destinationChild;
    if (sourceRow < destRow) {
        // Moving row(s) forward
        for (int i = 0; i < count; i++) {
            if (sourceRow < clrList.length()) {
                clrList.move(sourceRow, destRow);
            }
        }
    }
    else {
        // Moving row(s) backwards
        for (int i = count - 1; i >= 0; i--) {
            if (sourceRow + i < clrList.length()) {
                clrList.move(sourceRow + i, destRow);
            }
        }
    }
    endMoveRows();
    imgGen.NewPreviewImageNeeded();
    return true;
}

/** ****************************************************************************
 * @brief ClrListTableModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ClrListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        // Columns
        switch (section) {
        case colClrBox: return "Colour";
        case colClrHex: return "Hex";
        case colLoc: return "Pos";
        default:
            return QStringLiteral("Column %1").arg(section);
        }
    }
    else {
        // Rows
        return QString::asprintf("%d", section);
    }
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ****************************************************************************
 * @brief ColourMapEditorWidget::ColourMapEditorWidget
 * @param parent
 */
ColourMapEditorWidget::ColourMapEditorWidget(ImageGen& imgGenIn) :
    imgGen(imgGenIn), clrMap(&imgGenIn.colourMap), clrListModel(imgGenIn)
{
    this->setMinimumWidth(200);
    this->setMaximumWidth(500);
    // Create the widget
    // Sliders, then colour map display
    QVBoxLayout * clrMapLayout = new QVBoxLayout();
    this->setLayout(clrMapLayout);

    // Colour bar
    DrawColourBars(imgGen.genPreview);
    clrMapLayout->addWidget(&lblClrBarBase);
    clrMapLayout->addWidget(&lblClrBarMask);
    clrMapLayout->addWidget(&lblClrBarResult);
    clrMapLayout->addWidget(&maskChartView);
    maskChartView.setVisible(false);

    // Colourmap presets
    QHBoxLayout * layoutClrPresets = new QHBoxLayout();

    QPushButton * btnPresetJet = new QPushButton(QString("Jet"));
    QObject::connect(btnPresetJet, QPushButton::clicked, clrMap, ColourMap::SetPresetJet);
    layoutClrPresets->addWidget(btnPresetJet);

    QPushButton * btnPresetParula = new QPushButton(QString("Parula"));
    QObject::connect(btnPresetParula, QPushButton::clicked, clrMap, ColourMap::SetPresetParula);
    layoutClrPresets->addWidget(btnPresetParula);

    QPushButton * btnPresetHsv = new QPushButton(QString("Hsv"));
    QObject::connect(btnPresetHsv, QPushButton::clicked, clrMap, ColourMap::SetPresetHsv);
    layoutClrPresets->addWidget(btnPresetHsv);

    QPushButton * btnPresetHot = new QPushButton(QString("Hot"));
    QObject::connect(btnPresetHot, QPushButton::clicked, clrMap, ColourMap::SetPresetHot);
    layoutClrPresets->addWidget(btnPresetHot);

    QPushButton * btnPresetCool = new QPushButton(QString("Cool"));
    QObject::connect(btnPresetCool, QPushButton::clicked, clrMap, ColourMap::SetPresetCool);
    layoutClrPresets->addWidget(btnPresetCool);

    QPushButton * btnPresetBone = new QPushButton(QString("Bone"));
    QObject::connect(btnPresetBone, QPushButton::clicked, clrMap, ColourMap::SetPresetBone);
    layoutClrPresets->addWidget(btnPresetBone);

    clrMapLayout->addLayout(layoutClrPresets);

    // Table
    clrListTable.setModel(&clrListModel);
    clrListTable.setColumnWidth(clrListModel.colClrBox, 50);
    clrListTable.setColumnWidth(clrListModel.colClrHex, 60);
    clrListTable.setColumnWidth(clrListModel.colLoc, 40);
    clrMapLayout->addWidget(&clrListTable);

    QObject::connect(&clrListTable, ClrListTableView::clicked, &clrListModel, ClrListTableModel::TableClicked);

    // Buttons for add and delete
    QPushButton * btnAddRow = new QPushButton("Add");
    QPushButton * btnRemoveRow = new QPushButton("Remove");
    connect(btnAddRow, QPushButton::clicked, &clrListTable, ClrListTableView::AddRow);
    connect(btnRemoveRow, QPushButton::clicked, &clrListTable, ClrListTableView::RemoveSelectedRows);

    QHBoxLayout * layoutAddRemoveButtons = new QHBoxLayout();
    layoutAddRemoveButtons->addWidget(btnAddRow);
    layoutAddRemoveButtons->addWidget(btnRemoveRow);

    clrMapLayout->addLayout(layoutAddRemoveButtons);

    // ColourMap -> Model connections
    QObject::connect(clrMap, &ColourMap::PreColourListReset,
                     &clrListModel, &ClrListTableModel::PreColourListResetSlot);
    QObject::connect(clrMap, &ColourMap::PostColourListReset,
                     &clrListModel, &ClrListTableModel::PostColourListResetSlot);

    this->show();
}

/** ****************************************************************************
* @brief ColourMapEditorWidget::~ColourMapEditorWidget
*/
ColourMapEditorWidget::~ColourMapEditorWidget()
{

}

/** ************************************************************************ **/
void ColourMapEditorWidget::DrawColourBarsPreview()
{
    DrawColourBars(imgGen.genPreview);
}

/** ****************************************************************************
 * @brief ColourMapEditorWidget::DrawColourBars redraws the bar that demonstrates the colour map
 */
void ColourMapEditorWidget::DrawColourBars(GenSettings & genSet)
{
    barWidth = lblClrBarBase.width();
    QSize sizeClrBar(barWidth, heightClrBar);
    Rgb2D_C* dataBarBase = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    Rgb2D_C* dataBarMask = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    Rgb2D_C* dataBarResult = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    QVector<QPointF> chartData(sizeClrBar.width());

    // !@#$ Ensure that the indices are calculated here

    for (qint32 x = dataBarBase->xLeft, i = 0; x < dataBarBase->xLeft + dataBarBase->width; x++, i++) {
        qreal loc = (qreal)i / (qreal)sizeClrBar.width();
        // !@# Make this more efficient
        QRgb clrBase = clrMap->GetBaseColourValue(genSet, loc);
        qreal valMask = clrMap->GetMaskValue(genSet, loc);
        QRgb clrMask = qRgb(valMask*255, valMask*255, valMask*255);
        chartData[i] = QPointF(loc, valMask);
        QRgb clrResult = clrMap->GetBlendedColourValue(genSet, loc);

        for (int y = dataBarBase->yTop; y < dataBarBase->yTop + dataBarBase->height; y++) {
            dataBarBase->setPoint(x, y,  clrBase);
            dataBarMask->setPoint(x, y,  clrMask);
            dataBarResult->setPoint(x, y,  clrResult);
        }
    }

    imgBarBase = QImage((uchar*)dataBarBase->getDataPtr(), barWidth, sizeClrBar.height(), QImage::Format_ARGB32,
                       ImageDataDealloc, dataBarBase);

    imgBarMask = QImage((uchar*)dataBarMask->getDataPtr(), barWidth, sizeClrBar.height(), QImage::Format_ARGB32,
                       ImageDataDealloc, dataBarMask);

    imgBarResult = QImage((uchar*)dataBarResult->getDataPtr(), barWidth, sizeClrBar.height(), QImage::Format_ARGB32,
                       ImageDataDealloc, dataBarResult);

    lblClrBarBase.setPixmap(QPixmap::fromImage(imgBarBase));
    lblClrBarMask.setPixmap(QPixmap::fromImage(imgBarMask));
    lblClrBarResult.setPixmap(QPixmap::fromImage(imgBarResult));

    lblClrBarMask.setVisible(imgGen.s.maskCfg.enabled);
    lblClrBarResult.setVisible(imgGen.s.maskCfg.enabled);

    if (1) {
        // Plot RGB channels of the colour index
        // This makes it easier to check the colours are expected
        /*
        qint32 len = clrMap->clrIndexed.length();
        QVector<QPointF> clrR(len);
        QVector<QPointF> clrG(len);
        QVector<QPointF> clrB(len);
        for (int i = 0; i < len; i++) {
            qreal loc = (qreal)i / len;
            clrR[i] = QPointF(loc, clrMap->clrIndexed[i].redF() / 255.);
            clrG[i] = QPointF(loc, clrMap->clrIndexed[i].greenF() / 255.);
            clrB[i] = QPointF(loc, clrMap->clrIndexed[i].blueF() / 255.);
        }
        QLineSeries * clrRSeries = new QLineSeries();
        QLineSeries * clrGSeries = new QLineSeries();
        QLineSeries * clrBSeries = new QLineSeries();
        for (auto ser : maskChart.series()) { // Remove all series except maskSeries
            if (ser != maskSeries) {maskChart.removeSeries(ser);}
        }
        clrRSeries->replace(clrR);
        clrRSeries->setColor(Qt::red);
        maskChart.addSeries(clrRSeries);
        clrGSeries->replace(clrG);
        clrGSeries->setColor(Qt::darkGreen);
        maskChart.addSeries(clrGSeries);
        clrBSeries->replace(clrB);
        clrBSeries->setColor(Qt::blue);
        maskChart.addSeries(clrBSeries);
        */
    }

    // MASK CHART VIEW
    // The maskChartView has an annoying border of 9 pixels that prevents it
    // from lining up with the labels. I tried many methods to get rid of this,
    // but I eventually gave up.
    // The background of the mask is set to the dataBarResult
    if (!maskSeries) {
        maskSeries = new QLineSeries();
    }
    maskSeries->replace(chartData);
    maskChart.legend()->hide();
    if (!maskChart.series().contains(maskSeries)) {
        maskChart.addSeries(maskSeries);
    }

    maskChart.setBackgroundBrush(QBrush(imgBarResult));
    maskChart.setMargins(QMargins(0,0,0,0));
    maskChart.setBackgroundRoundness(0);
    maskSeries->setColor(Qt::gray);

    maskChart.createDefaultAxes();
    maskChart.axes(Qt::Horizontal)[0]->setRange(0, 1.0);
    maskChart.axes(Qt::Horizontal)[0]->setVisible(false);
    maskChart.axes(Qt::Vertical)[0]->setRange(0, 1.0);
    maskChart.axes(Qt::Vertical)[0]->setVisible(false);

    maskChartView.setChart(&maskChart);
    maskChartView.setFixedWidth(barWidth);
    maskChartView.setFixedHeight(heightClrBar * 3);
    maskChartView.setContentsMargins(0,0,0,0);
    maskChartView.setRenderHint(QPainter::Antialiasing);
    // qDebug("maskChartView.x=%d,   maskChart.x=%.2f. chartViewWidth=%d, lblColourWidth=%d", maskChartView.x(), maskChart.x(), maskChartView.width(), lblClrBarBase.width());
}


/** ****************************************************************************
 * @brief ColourMapEditorWidget::SetMaskChartVisible
 * @param on
 */
void ColourMapEditorWidget::SetMaskChartVisible(bool on)
{
    maskChartView.setVisible(on);
}

/**
 * @brief ColourMapEditorWidget::resizeEvent
 * @param event
 */
void ColourMapEditorWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (lblClrBarBase.width() != barWidth) {
        DrawColourBars(imgGen.genPreview);
    }
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/

/** ****************************************************************************
 * @brief ClrListTableView::keyPressEvent
 * @param event
 */
void ClrListTableView::RemoveSelectedRows()
{
    QModelIndexList selected = this->selectionModel()->selectedRows();
    if (selected.length() > 0) {
        qDebug("Deleting %d rows from colour fix @ %d", selected.length(), selected.first().row());
        // This function assumes that the selected rows are in ascending order
        for (int i = selected.length() - 1; i >= 0; i--) {
            this->model()->removeRow(selected[i].row());
        }
    }
}

/** ****************************************************************************
 * @brief ClrListTableView::AddRow
 */
void ClrListTableView::AddRow()
{
    this->model()->insertRow(this->model()->rowCount());
}

/** ****************************************************************************
 * @brief ClrListTableView::keyPressEvent
 * @param event
 */
void ClrListTableView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        RemoveSelectedRows();
        event->setAccepted(true);
    }
    else {
        event->setAccepted(false);
    }
}
