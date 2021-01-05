#include "colourmap.h"
#include "imagegen.h"
#include <QLayout>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QColorDialog>
#include <QHeaderView>
#include <QPushButton>



ColourMap colourMap;

/** ****************************************************************************
 * @brief ColourMap::ColourMap
 */
ColourMap::ColourMap()
{
    QObject::connect(this, ColourMap::RecalcSignal,
                     this, ColourMap::RecalcSlot, Qt::QueuedConnection);

    // !@# temporary data
    AddColour(ClrFix(Qt::green,   0.0));
    AddColour(ClrFix(Qt::cyan,  0.50));
    AddColour(ClrFix(Qt::blue, 1.00));

    CalcColourIndex();
    CalcMaskIndex();
}

/** ****************************************************************************
 * @brief ColourMap::ClrListChanged
 */
void ColourMap::ClrListChanged() {
    pendingRecalcClrIndex = true;
    emit RecalcSignal();
}

/** ****************************************************************************
 * @brief ColourMap::MaskSettingChanged
 */
void ColourMap::MaskSettingChanged() {
    pendingRecalcMaskIndex = true;
    emit RecalcSignal();
}

/** ****************************************************************************
 * @brief ColourMap::RecalcSlot
 */
void ColourMap::RecalcSlot()
{
    // This slot exists to prevent unnecessary extra recalculation
    if (pendingRecalcClrIndex) {
        CalcColourIndex();
    }
    if (pendingRecalcMaskIndex) {
        CalcMaskIndex();
    }
    if (pendingRecalcClrIndex || pendingRecalcMaskIndex) {
        emit NewClrMapReady();
    }
    pendingRecalcClrIndex = false;
    pendingRecalcMaskIndex = false;
}

/** ****************************************************************************
 * @brief ColourMap::SetMaskEnable
 * @param on
 * @returns true if the value was changed
 */
bool ColourMap::SetMaskEnable(bool on)
{
    if (m.enabled != on) {
        m.enabled = on;
        MaskSettingChanged();
        return true;
    }
    return false;
}

/** ****************************************************************************
 * @brief ColourMap::AddColour
 * @param clr
 * @param loc
 * @param listIdx
 */
void ColourMap::AddColour(QColor clr, qreal loc, qint32 listIdx)
{
    AddColour(ClrFix(clr, loc), listIdx);
    ClrListChanged();
}

void ColourMap::AddColour(ClrFix clrFix, qint32 listIdx)
{
    clrList.insert(listIdx, clrFix);
    ClrListChanged();
}

bool ColourMap::RemoveColour(qint32 listIdx)
{
    if (listIdx > clrList.length()) { return false; }
    if (clrList.length() <= 2) { return false; } // Must have at least 2 colours
    clrList.removeAt(listIdx);
    ClrListChanged();
    return true;
}

void ColourMap::EditColourLoc(qint32 listIdx, qreal newLoc)
{
    if (listIdx > clrList.length()) { return; }
    clrList[listIdx].loc = qBound(0., newLoc, 1.0);
    ClrListChanged();
}

void ColourMap::EditColour(qint32 listIdx, QRgb newClr)
{
    if (listIdx > clrList.length()) { return; }
    clrList[listIdx].clr = newClr;
    ClrListChanged();
}

/** ****************************************************************************
 * @brief ColourMap::SetPreset replaces the entire colour list with a preset list
 * @param preset
 */
void ColourMap::SetPreset(ClrMapPreset preset)
{
    this->beginResetModel();

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
    ClrListChanged();

    this->endResetModel();
}

/** ****************************************************************************
 * @brief ColourMap::GetColourValue calculates the colour, using the location and interpolation
 * @param loc is a number from 0 (min) to 1 (max)
 * @return a QRgb, using the alpha layer
 */
QRgb ColourMap::GetColourValue(qreal loc) const {
    qreal locAsIndex = (loc * (qreal)clrIndexMax);
    int idxBefore = std::min(clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;

    const QColor& before = clrIndexed[idxBefore];
    const QColor& after = clrIndexed[idxBefore + 1];
    return qRgba(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
                fa * before.blueF() + fb * after.blueF(),
                 255.0 * (fa * maskIndexed[idxBefore] + fb * maskIndexed[idxBefore + 1]));
}

/** ****************************************************************************
 * @brief ColourMap::GetBlendedColourValue calculates the colour, using the location and interpolation
 * @param loc is a number from 0 (min) to 1 (max)
 * @returns a QRgb with alpha layer off - the background is pre-blended into the colour
 */
QRgb ColourMap::GetBlendedColourValue(qreal loc) const {
    qreal locAsIndex = (loc * (qreal)clrIndexMax);
    int idxBefore = std::min(clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;

    const QColor& before = clrIndexed[idxBefore];
    const QColor& after = clrIndexed[idxBefore + 1];

    qreal mask = (fa * maskIndexed[idxBefore] + fb * maskIndexed[idxBefore + 1]);
    qreal maskInv = 1. - mask;
    return qRgb(mask * (fa * before.redF() + fb * after.redF()) + maskInv * m.backColour.redF(),
                mask * (fa * before.greenF() + fb * after.greenF()) + maskInv * m.backColour.greenF(),
                mask * (fa * before.blueF() + fb * after.blueF()) + maskInv * m.backColour.blueF());
}


/** ****************************************************************************
 * @brief ColourMap::GetBaseColourValue
 * @param loc
 * @return
 */
QRgb ColourMap::GetBaseColourValue(qreal loc) const
{
    qreal locAsIndex = (loc * (qreal)clrIndexMax);
    int idxBefore = std::min(clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;

    const QColor& before = clrIndexed[idxBefore];
    const QColor& after = clrIndexed[idxBefore + 1];
    return qRgb(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
                fa * before.blueF() + fb * after.blueF());
}

/** ****************************************************************************
 * @brief ColourMap::GetMaskValue
 * @param loc
 * @return
 */
qreal ColourMap::GetMaskValue(qreal loc) const
{
    if (!m.enabled) {return 1.0;}
    qreal locAsIndex = (loc * (qreal)clrIndexMax);
    int idxBefore = std::min(clrIndexMax-1, std::max(0,(int)locAsIndex));
    const qreal fb = (locAsIndex - idxBefore);
    const qreal fa = 1. - fb;
    return fa * maskIndexed[idxBefore] + fb * maskIndexed[idxBefore + 1];
}

/** ****************************************************************************
 * @brief ColourMap::GetClrFix
 * @param index
 * @return
 */
ClrFix ColourMap::GetClrFix(qint32 index) const
{
    if (index > clrList.length()) { return ClrFix(QRgb(), 0); }
    return clrList[index];
}

/** ****************************************************************************
 * @brief ColourMap::CalcColourIndex
 */
void ColourMap::CalcColourIndex()
{
    qDebug("CalcColourIndex");
    if (clrList.size() < 2) {
        qFatal("ColourMap::CreateIndexed() not enough colours in list!");
    }
    std::sort(clrList.begin(), clrList.end());


    // Calculate colour index
    clrIndexed.clear();
    clrIndexed.resize(clrIndexMax + 1);
    for (qint32 i = 0; i < clrIndexed.size(); i++) {
        qreal loc = (qreal)i / clrIndexMax;
        // Find the colours on either side
        ClrFix before(Qt::transparent, 0), after(Qt::transparent, 1.0);
        ClrFix thisClr(Qt::transparent, loc);
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
        clrIndexed[i] = Interpolate(loc, before, after);
    }
}

/** ****************************************************************************
 * @brief ColourMap::CalcMaskIndex
 */
void ColourMap::CalcMaskIndex()
{
    qDebug("CalcMaskIndex. Revs=%.2f, Duty=%.2f, Smooth=%.2f, Offset=%.2f", m.numRevs, m.dutyCycle, m.smooth, m.offset);
    qint32 maskLen = this->clrIndexMax + 1;
    maskIndexed.clear();
    maskIndexed.resize(maskLen);
    if (!m.enabled) {
        maskIndexed.fill(1.0);
        return;
    }

    qint32 period = std::max(2, (qint32)(maskLen/m.numRevs)); // period as #points
    qint32 highLen = qBound(1, (int) (m.dutyCycle * period), period);
    qint32 lowLen = period - highLen;
    qint32 transitionLen = std::max(1, (int)(m.smooth*0.5*maskLen/m.numRevs));

    // Pre-calculate the transition
    QVector<double> transition;
    transition.resize(transitionLen);
    if (transitionLen == 1) {transition[0] = 0;}
    else {
        double delta = 3.1415926 / (double) (transitionLen - 1);
        for (qint32 i = 0; i < transition.size(); i++) {
            transition[i] = cos(i * delta)*0.5 + 0.5; // 1 to 0
        }
    }
    // If the transition is greater than the period, pop values from
    // the back
    // Note this this can result in discontinuous lines. This is intentional;
    // To avoid discontinuities, I would seperately calculate the transition[]
    // for both high and low states. However, the discontinuities look pretty cool
    while (transitionLen * 2 > period) {
        transition.pop_back();
        transitionLen--;
    }

    qint32 activeTransIdx = m.offset * (qreal) period - transitionLen/2; // Transition start location (as an index). Should always be <= thisIdx
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
        if (thisTransIdx < transitionLen) {
            // Currently in a transition
            if (transVal) { // 0 to 1
                thisVal = 1.0 - transition[thisTransIdx];
            }
            else { // 1 to 0
                thisVal = transition[thisTransIdx];
            }
        }
        else {
            // Not in a transition. Constant value
            thisVal = transVal ? 1.0 : 0.0;
        }
        maskIndexed[thisIdx++] = thisVal;
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
int ColourMap::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return this->GetColourFixCount();
}

int ColourMap::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return colCount;
}

/** ****************************************************************************
 * @brief ColourMap::data
 * @param index
 * @param role
 * @return
 */
QVariant ColourMap::data(const QModelIndex &index, int role) const
{
    if (index.row() >= this->GetColourFixCount()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: // What is displayed when editing text
        if (index.column() == colClrHex) {
            const QColor& clr = this->GetClrFix(index.row()).clr;
            return QString::asprintf("%02X %02X %02X", clr.red(), clr.green(), clr.blue());
        }
        else if (index.column() == colLoc) {
            return QString::asprintf("%.1f", this->GetClrFix(index.row()).loc);
        }
        break;
    case Qt::BackgroundRole:
        if (index.column() == colClrBox) {
            return QBrush(this->GetClrFix(index.row()).clr);
        }
        break;
    }

    return QVariant();
}

/** ****************************************************************************
 * @brief ColourMap::setData
 * @param index
 * @param value
 * @param role
 * @return
 */
bool ColourMap::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index)) { return false; }
        if (index.column() == colLoc) {
            this->EditColourLoc(index.row(), value.toReal());
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

/** ****************************************************************************
 * @brief ColourMap::flags
 * @param index
 * @return
 */
Qt::ItemFlags ColourMap::flags(const QModelIndex &index) const
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
 * @brief ColourMap::TableClicked
 * @param index
 */
void ColourMap::TableClicked(const QModelIndex &index)
{
    if (index.column() == colClrBox) {
        QColor clrNew = QColorDialog::getColor(this->GetClrFix(index.row()).clr);
        if (!clrNew.isValid()) {return;} // User cancelled
        QRgb clrRgbNew = clrNew.rgb();
        this->EditColour(index.row(), clrRgbNew);

        emit dataChanged(index, index);
        QModelIndex idxHex = createIndex(index.row(), colClrHex);
        emit dataChanged(idxHex, idxHex);
    }
}

/** ****************************************************************************
 * @brief ColourMap::removeRows
 * @param startRow
 * @param rowCount
 * @param parent
 * @return
 */

/** ****************************************************************************
 * @brief ColourMap::removeRows
 */
bool ColourMap::removeRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        this->RemoveColour(startRow);
    }
    endRemoveRows();
    return true;
}

/** ****************************************************************************
 * @brief ColourMap::insertRows
 */
bool ColourMap::insertRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        this->AddColour(ClrFix(Qt::white, 100.0), startRow);
    }
    endInsertRows();
    return true;
}

/** ****************************************************************************
 * @brief ColourMap::moveRows
 */
bool ColourMap::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    int destRow = destinationChild;
    if (sourceRow < destRow) {
        // Moving row(s) forward
        for (int i = 0; i < count; i++) {
            this->MoveColour(sourceRow, destRow);
        }
    }
    else {
        // Moving row(s) backwards
        for (int i = count - 1; i >= 0; i--) {
            this->MoveColour(sourceRow + i, destRow);
        }
    }
    endMoveRows();
    return true;
}

/** ****************************************************************************
 * @brief ColourMap::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ColourMap::headerData(int section, Qt::Orientation orientation, int role) const
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
ColourMapEditorWidget::ColourMapEditorWidget(ColourMap* clrMapIn) : clrMap(clrMapIn)
{
    this->setMinimumWidth(200);
    this->setMaximumWidth(500);
    // Create the widget
    // Sliders, then colour map display
    QVBoxLayout * clrMapLayout = new QVBoxLayout();
    this->setLayout(clrMapLayout);

    // Colour bar
    DrawColourBars();
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
    tableClrFix.setModel(clrMap);
    tableClrFix.setColumnWidth(clrMap->colClrBox, 50);
    tableClrFix.setColumnWidth(clrMap->colClrHex, 60);
    tableClrFix.setColumnWidth(clrMap->colLoc, 40);
    clrMapLayout->addWidget(&tableClrFix);

    connect(&tableClrFix, ClrFixTableView::clicked, clrMap, ColourMap::TableClicked);
    connect(clrMap, ColourMap::NewClrMapReady, this, DrawColourBars, Qt::QueuedConnection);

    // Buttons for add and delete
    QPushButton * btnAddRow = new QPushButton("Add");
    QPushButton * btnRemoveRow = new QPushButton("Remove");
    connect(btnAddRow, QPushButton::clicked, &tableClrFix, ClrFixTableView::AddRow);
    connect(btnRemoveRow, QPushButton::clicked, &tableClrFix, ClrFixTableView::RemoveSelectedRows);

    QHBoxLayout * layoutAddRemoveButtons = new QHBoxLayout();
    layoutAddRemoveButtons->addWidget(btnAddRow);
    layoutAddRemoveButtons->addWidget(btnRemoveRow);

    clrMapLayout->addLayout(layoutAddRemoveButtons);

    this->show();
}

/** ****************************************************************************
* @brief ColourMapEditorWidget::~ColourMapEditorWidget
*/
ColourMapEditorWidget::~ColourMapEditorWidget()
{

}

/** ****************************************************************************
 * @brief ColourMapEditorWidget::DrawColourBars redraws the bar that demonstrates the colour map
 */
void ColourMapEditorWidget::DrawColourBars()
{
    barWidth = lblClrBarBase.width();
    QSize sizeClrBar(barWidth, heightClrBar);
    Rgb2D_C* dataBarBase = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    Rgb2D_C* dataBarMask = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    Rgb2D_C* dataBarResult = new Rgb2D_C(QPoint(0,0), sizeClrBar);
    QVector<QPointF> chartData(sizeClrBar.width());
    for (qint32 x = dataBarBase->xLeft, i = 0; x < dataBarBase->xLeft + dataBarBase->width; x++, i++) {
        qreal loc = (qreal)i / (qreal)sizeClrBar.width();
        // !@# Make this more efficient
        QRgb clrBase = clrMap->GetBaseColourValue(loc);
        qreal valMask = clrMap->GetMaskValue(loc);
        QRgb clrMask = qRgb(valMask*255, valMask*255, valMask*255);
        chartData[i] = QPointF(loc, valMask);
        QRgb clrResult = clrMap->GetBlendedColourValue(loc);

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

    lblClrBarMask.setVisible(clrMap->MaskIsEnabled());
    lblClrBarResult.setVisible(clrMap->MaskIsEnabled());

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
        DrawColourBars();
    }
}

/** ************************************************************************ **/
/** ************************************************************************ **/
/** ************************************************************************ **/

/** ****************************************************************************
 * @brief ClrFixTableView::keyPressEvent
 * @param event
 */
void ClrFixTableView::RemoveSelectedRows()
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
 * @brief ClrFixTableView::AddRow
 */
void ClrFixTableView::AddRow()
{
    this->model()->insertRow(this->model()->rowCount());
}

/** ****************************************************************************
 * @brief ClrFixTableView::keyPressEvent
 * @param event
 */
void ClrFixTableView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        RemoveSelectedRows();
        event->setAccepted(true);
    }
    else {
        event->setAccepted(false);
    }
}
