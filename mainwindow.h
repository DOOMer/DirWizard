#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QDir>
#include <QList>
#include <QCryptographicHash>
#ifdef MYPREFIX_DEBUG
#include <QDebug>
#endif
#include <QListWidgetItem>
#include <QCryptographicHash>
#include <QComboBox>
#include <QStringList>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileInfo>
#include <QList>
#include <QVector>
#include <QHash>
#include <QFile>
#include <QFileDialog>
#include <QDirIterator>
#include <QDir>
#include <QMutableListIterator>
#include <QListIterator>
#include <hashfileinfostruct.h>
#include <duplicatefinder.h>
#include <dircomparator.h>
#include <QThread>
#include <QComboBox>
#include <QTableWidget>
#include <QListWidget>
#include <QRegExp>
#include <QStringListIterator>
#include <QStringList>
#include <QMessageBox>
#include <QCloseEvent>
#include "calcandsavehash.h"
#include "loadandcheckhash.h"
#include "zipwalkchecker.h"

#include <QMimeData>
#include <QDragEnterEvent>

#include <QProcess>
#ifdef Q_OS_WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif
#include "filtersdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void saveModelDataToFile(QString filename);

private slots:
    void on_pushButton_Add_Dir_clicked();
    void on_pushButton_Cancel_clicked();
    void on_pushButton_Remove_Checked_clicked();
    void on_pushButton_Duplicate_Search_clicked();
    void on_pushButton_Remove_Dir_clicked();
    void on_AboutAction_Triggered(bool checked);
    // Duplicate files search
    void showDuplicatesInTable(QSharedPtrListHFIS itemsPtr);
    // Comparing Folders
    void showUniqFilesInTable(QSharedPtrListHFIS itemsPtr);
    void finishedThread();
    void on_pushButton_Compare_Folders_clicked();
    void on_pushButton_Save_From_Table_clicked();
    void on_pushButton_Calc_Hashes_clicked();
    void on_pushButton_Check_Hashes_clicked();
    void showInvalidHashFilesInTable(QSharedPtrListHFIS itemsPtr);
    void on_pushButton_Check_Zip_clicked();
    void showInvalidZipInTable(QSharedPtrListHFIS itemsPtr);
    // Custom popup menu for TableView
    void createCustomPopupMenuForTableView(const QPoint &pos);

    void on_setFiltersBtn_clicked();
    void on_maximum_files_for_progress_received(quint64 count);
    void on_current_processed_files_for_progress_received(quint64 count);

private:
    Ui::MainWindow *ui;
    QString dirNameForFolderDialog;
    QThread* thread;
    QStringList fileFilters;
#ifdef Q_OS_WIN32
    QWinTaskbarButton *buttonWinExtra;
    QWinTaskbarProgress *progressWinExtra;
#endif
    // ComboBox
    void addItemToComboBox(QString text, int data);
    void initHashComboBoxWidget();
    QCryptographicHash::Algorithm getCurrentHashAlgo();
    // ListWidget
    void initDirsListWidget();
    QList<QDir> getElementsFromDirsListWidget();
    void addToDirListWidget(const QString &dirName);
    // TableWidget
    void initTableWidget();
    // Duplicate files search
    void startDuplicateSearchInBackground();
    // Comparing Folders
    void startComparingFoldersInBackground();
    // Other
    void saveItemsToFile(const QString &fileName);
    void setUiPushButtonsEnabled(bool flag);
    // Calc hashes
    void startCalcHashesInBackground();
    void startCheckHashesInBackground();
    void startCheckZipsInBackground();

    // Drag Drop
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;

    void callBeforeBackgrowndWorkerStarted();
    // Filters
    bool useFilters() const;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e);
};

#endif // MAINWINDOW_H
