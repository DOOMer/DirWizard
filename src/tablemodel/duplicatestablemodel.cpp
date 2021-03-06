#include "duplicatestablemodel.h"

namespace {
const int MaxColumns = 5;
QColor firstColor = QColor(Qt::white);
QColor secondColor = QColor(Qt::green);

QString getFileNameShort(const QString &path)
{
#ifdef NATIVE_PATH_SEP
    QString fname = QDir::toNativeSeparators(path);
    return  fname.section(QDir::separator(), -1);
#else
    return  path.section('/', -1);
#endif
}
}
/*
typedef struct {
    //QFileInfo i;
    QString fileName;
    QString hash;
    bool checked;
    uint groupID;
    qint64 size;
} HashFileInfoStruct;
*/

int DuplicatesTableModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : items->count();
}


int DuplicatesTableModel::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : MaxColumns;
}

QVariant DuplicatesTableModel::headerData(int section,
                                          Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case Column::checked: return tr("Remove?"); break;
        case Column::fileName: return tr("FileName"); break;
        case Column::hash: return tr("Hash");  break;
        case Column::groupId: return tr("groupId"); break;
        case Column::size: return tr("Size"); break;
        default: Q_ASSERT(false);
        }
    }
    return section + 1;
}

QVariant DuplicatesTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
            index.row() < 0 || index.row() >= items->count() ||
            index.column() < 0 || index.column() >= MaxColumns)
        return QVariant();
    const HashFileInfoStruct &item = items->at(index.row());
    switch(role) {
    case Qt::SizeHintRole: {
        QStyleOptionComboBox option;
        switch (index.column()) {
        case Column::checked: option.currentText = QString(""); break;
        case Column::fileName: option.currentText = getFileNameShort(item.fileName); break;
        case Column::hash: option.currentText = item.hash; break;
        case Column::groupId: option.currentText = item.groupID; break;
        case Column::size: option.currentText = QString::number(item.size); break;
        default: Q_ASSERT(false);
        }
        QFontMetrics fontMetrics(data(index, Qt::FontRole)
                                 .value<QFont>());
        option.fontMetrics = fontMetrics;
        QSize size(fontMetrics.width(option.currentText),
                   fontMetrics.height());
        return qApp->style()->sizeFromContents(QStyle::CT_ComboBox,
                                               &option, size);
    }
    case Qt::DisplayRole: {
        switch (index.column()) {
        case Column::checked: return QVariant();
        case Column::fileName: return getFileNameShort(item.fileName);
        case Column::hash: return item.hash;
        case Column::groupId: return item.groupID;
        case Column::size: return item.size;
        default: Q_ASSERT(false);
        }
    }
    case Qt::EditRole: {
        return QVariant();
    }
    case Qt::CheckStateRole: {
        if(index.column() == Column::checked) {
            return item.checked==true ? Qt::Checked : Qt::Unchecked;
        }
        return QVariant();
    }
    case Qt::ToolTipRole: {
        switch (index.column()) {
        case Column::checked: return tr("Remove?");
        case Column::fileName: return item.fileName;
        case Column::hash: return item.hash;
        case Column::groupId: return item.groupID;
        case Column::size: return item.size;
        default: Q_ASSERT(false);
        }
    }
    case Qt::BackgroundColorRole: {
        QColor color = item.groupID % 2 == 0? firstColor : secondColor;
        return QVariant(color);
    }
    default: return QVariant();
    }
    return QVariant();
}

bool DuplicatesTableModel::setData(const QModelIndex &index,
                                   const QVariant &value, int role)
{
    const int column = index.column();
    const int row = index.row();
    if (!index.isValid() ||
            row < 0 || row >= items->count() ||
            column < 0 || column > MaxColumns)
        return false;
    HashFileInfoStruct &item = (*items.data())[row];
    if (role == Qt::CheckStateRole)
    {
        if (value.toInt() == Qt::Checked)
        {
            item.checked = true;
        }
        else {
            item.checked = false;
        }
        emit dataChanged(index, index);
    }
    return false;
}

void DuplicatesTableModel::sort(int column, Qt::SortOrder order)
{
#ifdef MYPREFIX_DEBUG
    if(order == Qt::AscendingOrder) {
        qDebug()<< "DuplicatesTableModel::sort Qt::AscendingOrder";
    } else {
        qDebug()<< "DuplicatesTableModel::sort Qt::DescendingOrder";
    }
#endif
    switch(column) {
    case Column::checked:
        std::sort(items->begin(), items->end(), [](const HashFileInfoStruct &v1, const HashFileInfoStruct &v2)->bool {
            if(v1.checked && !v2.checked)
                return true;
            return false;
        }); break;
    case Column::fileName:
        std::sort(items->begin(), items->end(), [](const HashFileInfoStruct &v1, const HashFileInfoStruct &v2)->bool {
            return v1.fileName > v2.fileName;
        });
        break;
    case Column::hash:
        std::sort(items->begin(), items->end(), [](const HashFileInfoStruct &v1, const HashFileInfoStruct &v2)->bool {
            return v1.hash > v2.hash;
        });
        break;
    case Column::groupId:
        std::sort(items->begin(), items->end(), [](const HashFileInfoStruct &v1, const HashFileInfoStruct &v2)->bool {
            return v1.groupID > v2.groupID;
        });
        break;
    case Column::size:
        std::sort(items->begin(), items->end(), [](const HashFileInfoStruct &v1, const HashFileInfoStruct &v2)->bool {
            return v1.size > v2.size;
        });
        break;
    default: return;
    }
    if(order != Qt::AscendingOrder)
        std::reverse(items->begin(), items->end());
    emit dataChanged(QModelIndex(), QModelIndex());
}

QString DuplicatesTableModel::getFileName(const QModelIndex &index) const
{
    if (!index.isValid() ||
            index.row() < 0 || index.row() >= items->count() ||
            index.column() < 0 || index.column() >= MaxColumns)
        return "";
    const HashFileInfoStruct &item = items->at(index.row());
    return item.fileName;
}
