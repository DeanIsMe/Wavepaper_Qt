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
    QObject::connect(this, ColourMap::ClrListChanged,
                     this, ColourMap::CreateIndex, Qt::QueuedConnection);
    // !@# temporary data
    AddColour(ClrFix(Qt::green,   0));
    AddColour(ClrFix(Qt::black,  25));
    AddColour(ClrFix(Qt::red,  50));
    AddColour(ClrFix(Qt::black,  75));
    AddColour(ClrFix(Qt::blue, 100));
    CreateIndex();
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
    emit ClrListChanged();
}

void ColourMap::AddColour(ClrFix clrFix, qint32 listIdx)
{
    clrList.insert(listIdx, clrFix);
    emit ClrListChanged();
}

bool ColourMap::RemoveColour(qint32 listIdx)
{
    if (listIdx > clrList.length()) { return false; }
    if (clrList.length() <= 2) { return false; } // Must have at least 2 colours
    clrList.removeAt(listIdx);
    emit ClrListChanged();
    return true;
}

void ColourMap::EditColourLoc(qint32 listIdx, qreal newLoc)
{
    if (listIdx > clrList.length()) { return; }
    clrList[listIdx].loc = qBound(0., newLoc, 100.);
    emit ClrListChanged();
}

void ColourMap::EditColour(qint32 listIdx, QRgb newClr)
{
    if (listIdx > clrList.length()) { return; }
    clrList[listIdx].clr = newClr;
    emit ClrListChanged();
}

/** ****************************************************************************
 * @brief ColourMap::GetColourValue calculates the colour, using the index and interpolation
 * @param loc is a number from 0 (min) to 100 (max)
 * @return
 */
QRgb ColourMap::GetColourValue(qreal loc) const {
    int locBefore = std::min(99, std::max(0,(int)loc));
    const qreal fb = (loc - locBefore);
    const qreal fa = 1. - fb;

    const QColor& before = clrIndexed[locBefore];
    const QColor& after = clrIndexed[locBefore + 1];
    return qRgb(fa * before.redF() + fb * after.redF(),
             fa * before.greenF() + fb * after.greenF(),
                fa * before.blueF() + fb * after.blueF());
}

ClrFix ColourMap::GetClrFix(qint32 index) const
{
    if (index > clrList.length()) { return ClrFix(QRgb(), 0); }
    return clrList[index];
}

/** ****************************************************************************
 * @brief ColourMap::CreateIndex
 */
void ColourMap::CreateIndex()
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
    emit NewClrMapReady();
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


ClrFixModel::ClrFixModel(ColourMap * clrMapIn) : clrMap(clrMapIn)
{

}

int ClrFixModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return clrMap->GetColourFixCount();
}

int ClrFixModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

/** ****************************************************************************
 * @brief ClrFixModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ClrFixModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= clrMap->GetColourFixCount()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: // What is displayed when editing text
        if (index.column() == colClrHex) {
            const QColor& clr = clrMap->GetClrFix(index.row()).clr;
            return QString::asprintf("%02X %02X %02X", clr.red(), clr.green(), clr.blue());
        }
        else if (index.column() == colLoc) {
            return QString::asprintf("%.1f", clrMap->GetClrFix(index.row()).loc);
        }
        break;
    case Qt::BackgroundRole:
        if (index.column() == colClrBox) {
            return QBrush(clrMap->GetClrFix(index.row()).clr);
        }
        break;
    }

    return QVariant();
}

/** ****************************************************************************
 * @brief ClrFixModel::setData
 * @param index
 * @param value
 * @param role
 * @return
 */
bool ClrFixModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index)) { return false; }
        if (index.column() == colLoc) {
            clrMap->EditColourLoc(index.row(), value.toReal());
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

/** ****************************************************************************
 * @brief ClrFixModel::flags
 * @param index
 * @return
 */
Qt::ItemFlags ClrFixModel::flags(const QModelIndex &index) const
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
 * @brief ClrFixModel::TableClicked
 * @param index
 */
void ClrFixModel::TableClicked(const QModelIndex &index)
{
    if (index.column() == colClrBox) {
        QColor clrNew = QColorDialog::getColor(clrMap->GetClrFix(index.row()).clr);
        if (!clrNew.isValid()) {return;} // User cancelled
        QRgb clrRgbNew = clrNew.rgb();
        clrMap->EditColour(index.row(), clrRgbNew);

        emit dataChanged(index, index);
        QModelIndex idxHex = createIndex(index.row(), colClrHex);
        emit dataChanged(idxHex, idxHex);
    }
}

/** ****************************************************************************
 * @brief ClrFixModel::removeRows
 * @param startRow
 * @param rowCount
 * @param parent
 * @return
 */

/** ****************************************************************************
 * @brief ClrFixModel::removeRows
 */
bool ClrFixModel::removeRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        clrMap->RemoveColour(startRow);
    }
    endRemoveRows();
    return true;
}

/** ****************************************************************************
 * @brief ClrFixModel::insertRows
 */
bool ClrFixModel::insertRows(int startRow, int rowCount, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), startRow, startRow+rowCount-1);
    for (int i = 0; i < rowCount; i++) {
        clrMap->AddColour(ClrFix(Qt::white, 100.0), startRow);
    }
    endInsertRows();
    return true;
}

/** ****************************************************************************
 * @brief ClrFixModel::moveRows
 */
bool ClrFixModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    int destRow = destinationChild;
    if (sourceRow < destRow) {
        // Moving row(s) forward
        for (int i = 0; i < count; i++) {
            clrMap->MoveColour(sourceRow, destRow);
        }
    }
    else {
        // Moving row(s) backwards
        for (int i = count - 1; i >= 0; i--) {
            clrMap->MoveColour(sourceRow + i, destRow);
        }
    }
    endMoveRows();
    return true;
}

/** ****************************************************************************
 * @brief ClrFixModel::headerData
 * @param section
 * @param orientation
 * @param role
 * @return
 */
QVariant ClrFixModel::headerData(int section, Qt::Orientation orientation, int role) const
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


/** ****************************************************************************
 * @brief ColourMapEditorWidget::ColourMapEditorWidget
 * @param parent
 */
ColourMapEditorWidget::ColourMapEditorWidget(QWidget *parent) : modelClrFix(&colourMap)
{
    Q_UNUSED(parent);
    this->setMinimumWidth(200);
    this->setMaximumWidth(500);
    // Create the widget
    // Sliders, then colour map display
    QVBoxLayout * clrMapLayout = new QVBoxLayout();
    this->setLayout(clrMapLayout);

    // Colour bar
    DrawColourBar();

    clrMapLayout->addWidget(&lblClrBar);

    // Table
    tableClrFix.setModel(&modelClrFix);
    tableClrFix.setColumnWidth(modelClrFix.colClrBox, 50);
    tableClrFix.setColumnWidth(modelClrFix.colClrHex, 60);
    tableClrFix.setColumnWidth(modelClrFix.colLoc, 40);
    clrMapLayout->addWidget(&tableClrFix);

    connect(&tableClrFix, ClrFixTableView::clicked, &modelClrFix, ClrFixModel::TableClicked);
    connect(&colourMap, ColourMap::NewClrMapReady, this, DrawColourBar);

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

ColourMapEditorWidget::~ColourMapEditorWidget()
{

}

/**
 * @brief ColourMapEditorWidget::DrawColourBar redraws the bar that demonstrates the colour map
 */
void ColourMapEditorWidget::DrawColourBar()
{
    QSize sizeClrBar(this->width(), heightClrBar);
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
}


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

void ClrFixTableView::AddRow()
{
    this->model()->insertRow(this->model()->rowCount());
}

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
