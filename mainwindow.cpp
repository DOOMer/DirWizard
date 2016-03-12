#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dirNameForFolderDialog(QDir::current().dirName()),
    thread(new QThread(this)),
    itemsResult(nullptr)
{
    ui->setupUi(this);
    initDirsListWidget();
    initHashComboBoxWidget();
    initTableWidget();
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::on_AboutAction_Triggered);
    QObject::connect(ui->pushButton_Calc_Hashes, &QPushButton::clicked, this, &MainWindow::on_pushButton_Calc_Hashes_clicked);
    //QObject::connect(ui->pushButton_Check_Hashes, &QPushButton::clicked, this, &MainWindow::on_pushButton_Check_Hashes_clicked);
    //QObject::connect(ui->pushButton_Check_Zip, &QPushButton::clicked, this, &MainWindow::on_pushButton_Check_Zip_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Dirs List Widget Start
void MainWindow::initDirsListWidget()
{
   ui->listWidget->clear();
}

QList<QDir> MainWindow::getElementsFromDirsListWidget()
{
    int count = ui->listWidget->count();
    QListWidgetItem *item;
    QList<QDir> dirList;
    dirList.reserve(count);
    QDir dir;

    QString filterS = ui->lineEdit_Filter->text();
    QStringList qsl;
    if(!filterS.isNull() && !filterS.isEmpty())
    {
        qsl = filterS.split(QRegExp("\\s"));
#ifdef MYPREFIX_DEBUG
        QStringListIterator i(qsl);
        while(i.hasNext())
            qDebug()<< i.next();
#endif
    }

    QListWidget *list = ui->listWidget;
    for(int i=0; i < count; ++i)
    {
        item = list->item(i);
        dir = QDir(item->data(Qt::UserRole).toString());
        if(!qsl.empty()){
            dir.setNameFilters(qsl);
        }
        dirList.append(dir);
    }
    return dirList;
}

void MainWindow::addToDirListWidget(const QString &dirName)
{
    QListWidgetItem *item = new QListWidgetItem(QDir(dirName).absolutePath());
    QVariant variant(dirName);
    item->setData(Qt::UserRole, variant);
    item->setToolTip(dirName);
    ui->listWidget->addItem(item);
}
// END

// Hashes ComboBox
void MainWindow::initHashComboBoxWidget()
{
    addItemToComboBox(QString("Md5"), QCryptographicHash::Md5);
    addItemToComboBox(QString("Sha1"), QCryptographicHash::Sha1);
    addItemToComboBox(QString("Sha256"), QCryptographicHash::Sha256);
    addItemToComboBox(QString("Sha512"), QCryptographicHash::Sha512);
    addItemToComboBox(QString("Sha3_256"), QCryptographicHash::Sha3_256);
    addItemToComboBox(QString("Sha3_512"), QCryptographicHash::Sha3_512);
}

void MainWindow::addItemToComboBox(QString text, int data)
{
    ui->comboBox->addItem(text, QVariant(data));
}

QCryptographicHash::Algorithm MainWindow::getCurrentHashAlgo()
{
    QVariant variant = ui->comboBox->currentData();
    return static_cast<QCryptographicHash::Algorithm>(variant.toUInt());
}
// END


// Table Widget
void MainWindow::initTableWidget()
{
    QStringList horizontalHeader;
    horizontalHeader.append(tr("Remove?"));
    horizontalHeader.append(tr("groupId"));
    horizontalHeader.append(tr("Hash"));
    horizontalHeader.append(tr("Size"));
    horizontalHeader.append(tr("FileName"));
    int columnCount = horizontalHeader.count();
    int rowCount = 0;
    QTableWidget *table = ui->tableWidget;
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);
    // Set alternating row colors from now to future.
    table->setAlternatingRowColors(true);
    table->setStyleSheet("alternate-background-color: grey;background-color: white;");
}

void MainWindow::showDuplicatesInTable(QSharedPtrListHFIS itemsPtr)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "showDuplicatesInTable";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append(tr("Remove?"));
    horizontalHeader.append(tr("groupId"));
    horizontalHeader.append(tr("Hash"));
    horizontalHeader.append(tr("Size"));
    horizontalHeader.append(tr("FileName"));
    int columnCount = horizontalHeader.count();
    const int rowCount = itemsPtr.data()->count();
    QTableWidgetItem* tableItem;

    QTableWidget *table = ui->tableWidget;
    QObject::disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::disconnect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);
    itemsResult = itemsPtr;
    QListIterator<HashFileInfoStruct> itemIt(*itemsPtr.data());
    HashFileInfoStruct file;
    int row = 0;
    QString text;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        text = "";
        tableItem = new QTableWidgetItem(text);
        tableItem->setText(text);
        tableItem->setFlags(tableItem->flags() | Qt::ItemIsUserCheckable);
        if(file.checked)
            tableItem->setCheckState(Qt::Checked);
        else
            tableItem->setCheckState(Qt::Unchecked);
        tableItem->setToolTip(file.fileName);
        table->setItem(row, 0, tableItem);

        text = QString("%1").arg(file.groupID);
        tableItem = new QTableWidgetItem(text);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(text);
        table->setItem(row, 1, tableItem);

        tableItem = new QTableWidgetItem(file.hash);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.hash);
        table->setItem(row, 2, tableItem);

        text = QString("%1").arg(file.size);
        tableItem = new QTableWidgetItem(text);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(text);
        table->setItem(row, 3, tableItem);

        tableItem = new QTableWidgetItem(file.fileName);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.fileName);
        table->setItem(row, 4, tableItem);
        ++row;
    }
    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
    QObject::connect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}
/*
QList<QString> MainWindow::getCheckedFileNamesFormTable()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "getCheckedFileNamesFormTable";
#endif
    QList<QString> result;
    int rowCount = ui->tableWidget->rowCount();
    QTableWidgetItem *item;
    QTableWidget *table = ui->tableWidget;
    for(int row=0; row<rowCount; row++)
    {
        item = table->item(row, 0);
        if( item->checkState() == Qt::Checked )
        {
            QVariant data = item->data(Qt::UserRole);
            result.append(data.toString());
        }
    }
    return result;
}
*/
void MainWindow::tableWidgetItemChanged(QTableWidgetItem * item)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::tableWidgetItemChanged";
#endif
    if(itemsResult == nullptr) return;
    //if(itemsResult->isEmpty()) return;
    QString fileName = item->toolTip();
    QMutableListIterator<HashFileInfoStruct> it(*itemsResult.data());
    HashFileInfoStruct strct;
    while(it.hasNext())
    {
        strct = it.next();
        if(fileName.compare(strct.fileName, Qt::CaseSensitive) == 0)
        {
            if(item->checkState() == Qt::Checked)
            strct.checked = true;
            else strct.checked = false;
            it.setValue(strct);
            break;
        }
    }
}
// END

// Duplicate Files Search
void MainWindow::startDuplicateSearchInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startDuplicateSearchInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty() && !thread->isRunning())
    {
        DuplicateFinder *worker = new DuplicateFinder(getCurrentHashAlgo(), nullptr);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        //connect(thread, SIGNAL(started()), worker, SLOT(process()));
        QObject::connect(thread, &QThread::started, worker, &DuplicateFinder::process);
        //connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        /*No examples where QThread::finished connected to &QThread::quit !
         * And then we need Worker::finished. */
        QObject::connect(worker, &DuplicateFinder::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, worker, &DuplicateFinder::deleteLater);//From Off documentation
        QObject::connect(worker, &DuplicateFinder::finished, this, &MainWindow::finishedThread);
        //connect(worker, SIGNAL(finishedWData(QList<HashFileInfoStruct> *)), this, SLOT(showDuplicatesInTable(QList<HashFileInfoStruct> *)));
        QObject::connect(worker, &DuplicateFinder::finishedWData, this, &MainWindow::showDuplicatesInTable);
#ifdef MYPREFIX_DEBUG
        qDebug() << "startThread";
#endif
        thread->start();
    }
}

// End

// Complete folders
/*
 *
*/
void MainWindow::showUniqFilesInTable(QSharedPtrListHFIS itemsPtr)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "showUniqFilesInTable";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append(tr("File names for unique files"));
    int columnCount = horizontalHeader.count();
    const int rowCount = itemsPtr.data()->count();
    QTableWidgetItem* tableItem;

    QTableWidget *table = ui->tableWidget;
    QObject::disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::disconnect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);

    itemsResult=itemsPtr;
qDebug() << "Num of unique" << itemsPtr.data()->count();
    QListIterator<HashFileInfoStruct> itemIt(*itemsPtr.data());
    HashFileInfoStruct file;
    int row = 0;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        tableItem = new QTableWidgetItem(file.fileName);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.fileName);
        table->setItem(row, 0, tableItem);
        ++row;
    }

    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
    QObject::connect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::startComparingFoldersInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startComparingFoldersInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty() && !thread->isRunning())
    {
        DirComparator *worker = new DirComparator(getCurrentHashAlgo(), nullptr);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        //connect(thread, SIGNAL(started()), worker, SLOT(process()));
        QObject::connect(thread, &QThread::started, worker, &DirComparator::process);
        //connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, &DirComparator::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, worker, &DirComparator::deleteLater);//From Off documentation
        QObject::connect(worker, &DirComparator::finished, this, &MainWindow::finishedThread);
        //connect(worker, SIGNAL(finishedWData(QList<HashFileInfoStruct> *)), this, SLOT(compareFoldersComplete(QList<HashFileInfoStruct> *)));
        QObject::connect(worker, &DirComparator::finishedWData, this, &MainWindow::showUniqFilesInTable);
#ifdef MYPREFIX_DEBUG
        qDebug() << "startThread";
#endif
        thread->start();
    }
}
// END

/*
 * Open Folder Choose dialog and add Dirname to ListWidget
 */
void MainWindow::on_pushButton_Add_Dir_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "on_pushButton_Add_Dir_clicked";
#endif
    QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 dirNameForFolderDialog,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(!dirName.isEmpty() && !dirName.isNull())
    {
        dirNameForFolderDialog = QFileInfo(dirName).absoluteDir().absolutePath();
        addToDirListWidget(dirName);
    }
}

/*
 * Remove Seleccted item from TableWidget
 */
void MainWindow::on_pushButton_Remove_Checked_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << ":on_pushButton_Remove_Checked_clicked";
#endif
    /*
    QList<QString> dirs = getCheckedFileNamesFormTable();
    if(!dirs.isEmpty())
    {
        QString dir;
        QListIterator<QString> it(dirs);
        dir = it.next();
        QDir(dir).remove(dir);
    }
    */
    if(itemsResult == nullptr) return;
    //if(itemsResult->isEmpty()) return;
    QMutableListIterator<HashFileInfoStruct> it(*itemsResult.data());
    HashFileInfoStruct strct;
    int removed = 0;
    while(it.hasNext())
    {
        strct = it.next();
        if(strct.checked)
        {
            if(QDir(strct.fileName).remove(strct.fileName)) {
                ++removed;
                it.remove();
            } else {
                QMessageBox::warning(this,
                                     "DirWizard",
                                     tr("Can't delete file: %1").arg(strct.fileName));
            }
        }
    }
    QTableWidget *table = ui->tableWidget;
    QObject::disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    int rowCount = table->rowCount();
    QTableWidgetItem *item;
    QVector<int> qv;
    qv.reserve(removed);
    for(int row=0; row < rowCount; ++row)
    {
        item = table->item(row, 0);
        if( item->checkState() == Qt::Checked )
        {
            qv.append(row);
        }
    }
    QVectorIterator<int> i(qv);
    while(i.hasNext())
        table->removeRow(i.next());
    QObject::connect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QMessageBox::information(this,
                             "DirWizard",
                             tr("Files have been deleted!"));
}

/*
 * Slot for Start Duplicate file Search in background (other function)
 */
void MainWindow::on_pushButton_Duplicate_Search_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << "\n";
    qDebug() << "QThread: " << thread << "\n";
#endif
    if (thread->isRunning()) {
        thread->wait();
    }
    //ClearItemsResultStore();
    startDuplicateSearchInBackground();
    setUiPushButtonsEnabled(!thread->isRunning());
}

/*
 * Remove Seleccted item (Dir Name) from ListWidget
 */
void MainWindow::on_pushButton_Remove_Dir_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "on_pushButton_RemoveDir_clicked";
#endif
    // Din't work
    //ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
    delete ui->listWidget->currentItem();
}

void MainWindow::on_pushButton_Cancel_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "on_pushButton_Cancel_clicked";
#endif
    thread->requestInterruption();
}

void MainWindow::on_AboutAction_Triggered(bool checked)
{
    Q_UNUSED(checked)
    QMessageBox::about(this,
                       tr("About DirWizard"),
                       tr("<h2>DirWizard</h2>"
                          "<p>Written by Yuriy (Yuri) Astrov<br/>"
                          "Based on QT 5<br/>"
                          "Licensed by GPLv2<br/>"
                          "Version: %1<br/>"
                          "URL: <a href='%2'>%2</a><p>").arg(APP_VERSION, APP_URL));

 }

void MainWindow::finishedThread()
{
    setUiPushButtonsEnabled(true);
    QMessageBox::information(this,
                             "DirWizard",
                             tr("Task completed successfully!"));
}


void MainWindow::on_pushButton_Compare_Folders_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << "\n";
    qDebug() << "QThread: " << thread << "\n";
#endif
    if (thread->isRunning()) {
        thread->wait();
    }
    //ClearItemsResultStore();
    startComparingFoldersInBackground();
    setUiPushButtonsEnabled(!thread->isRunning());
}

// Save To File
void MainWindow::saveItemsToFile(const QString &fileName)
{
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(false);
        HashFileInfoStruct s;
        QString delimeter = "\t";
        QString endOfLine = "\n";

        out << tr("Group ID") << delimeter;
        out << tr("Size") << delimeter;
        out << tr("Hash") << delimeter << tr("FileName");
        out << endOfLine;

        QListIterator<HashFileInfoStruct> it(*itemsResult);
        while(it.hasNext())
        {
            s = it.next();
            out << QString("%1").arg(s.groupID) << delimeter;
            out << s.size << delimeter << s.hash << delimeter << s.fileName << endOfLine;
        }
        file.close();
    }
}

void MainWindow::on_pushButton_Save_From_Table_clicked()
{
    if(itemsResult != nullptr)
    {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                     QDir::homePath(),
                                     tr("Text files (*.txt)"));
    if(!fileName.isNull() && !fileName.isEmpty())
    {
        QFileInfo qF(fileName);
        if(qF.exists())
        {
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                      "DirWizard",
                                                                      tr("File already exists!\nOverwrite?"),
                                                                      QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
            {
                return;
            }
        }
        saveItemsToFile(fileName);
    }
    }
}

/*
    Calc Hashes
*/
void MainWindow::startCalcHashesInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startCalcHashesInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty() && !thread->isRunning())
    {
        CalcAndSaveHash *worker = new CalcAndSaveHash(getCurrentHashAlgo(), nullptr);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        QObject::connect(thread, &QThread::started, worker, &CalcAndSaveHash::process);
        QObject::connect(worker, &CalcAndSaveHash::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, worker, &CalcAndSaveHash::deleteLater);//From Off documentation
        QObject::connect(worker, &CalcAndSaveHash::finished, this, &MainWindow::finishedThread);
#ifdef MYPREFIX_DEBUG
        qDebug() << "startThread";
#endif
        thread->start();
    }
}

void MainWindow::on_pushButton_Calc_Hashes_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << "\n";
    qDebug() << "QThread: " << thread << "\n";
#endif
    if (thread->isRunning()) {
        thread->wait();
    }
    startCalcHashesInBackground();
    setUiPushButtonsEnabled(!thread->isRunning());
}

/*
Checked Hashes
*/
void MainWindow::on_pushButton_Check_Hashes_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << __PRETTY_FUNCTION__ << "\n";
    qDebug() << "QThread: " << thread << "\n";
#endif
    if (thread->isRunning()) {
        thread->wait();
    }
    startCheckHashesInBackground();
    setUiPushButtonsEnabled(!thread->isRunning());
}

void MainWindow::startCheckHashesInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startCheckHashesInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty() && !thread->isRunning())
    {
        LoadAndCheckHash *worker = new LoadAndCheckHash(getCurrentHashAlgo(), nullptr);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        QObject::connect(thread, &QThread::started, worker, &LoadAndCheckHash::process);
        QObject::connect(worker, &LoadAndCheckHash::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, worker, &LoadAndCheckHash::deleteLater);//From Off documentation
        QObject::connect(worker, &LoadAndCheckHash::finished, this, &MainWindow::finishedThread);
        QObject::connect(worker, &LoadAndCheckHash::finishedWData, this, &MainWindow::showInvalidHashFilesInTable);
#ifdef MYPREFIX_DEBUG
        qDebug() << "startThread";
#endif
        thread->start();
    }
}

void MainWindow::showInvalidHashFilesInTable(QSharedPtrListHFIS itemsPtr)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::showInvalidHashFilesInTable";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append(tr("Remove?"));
    horizontalHeader.append(tr("Files with other hashes"));
    int columnCount = horizontalHeader.count();
    const int rowCount = itemsPtr.data()->count();
    QTableWidgetItem* tableItem;

    itemsResult=itemsPtr;

    QTableWidget *table = ui->tableWidget;
    QObject::disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::disconnect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);

    QListIterator<HashFileInfoStruct> itemIt(*itemsPtr.data());
    HashFileInfoStruct file;
    int row = 0;
    QString text;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        text = "";
        tableItem = new QTableWidgetItem(text);
        tableItem->setText(text);
        tableItem->setFlags(tableItem->flags() | Qt::ItemIsUserCheckable);
        if(file.checked)
            tableItem->setCheckState(Qt::Checked);
        else
            tableItem->setCheckState(Qt::Unchecked);
        tableItem->setToolTip(file.fileName);
        table->setItem(row, 0, tableItem);

        tableItem = new QTableWidgetItem(file.fileName);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.fileName);
        table->setItem(row, 1, tableItem);
        ++row;
    }

    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
    QObject::connect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::on_pushButton_Check_Zip_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::on_pushButton_Check_Zip_clicked";
#endif
    //ClearItemsResultStore();
    startCheckZipsInBackground();
    setUiPushButtonsEnabled(!thread->isRunning());
}

void MainWindow::startCheckZipsInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::startCheckZipsInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty() && !thread->isRunning())
    {
        ZipWalkChecker *worker = new ZipWalkChecker(nullptr);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        QObject::connect(thread, &QThread::started, worker, &ZipWalkChecker::process);
        QObject::connect(worker, &ZipWalkChecker::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, worker, &ZipWalkChecker::deleteLater);//From Off documentation
        QObject::connect(worker, &ZipWalkChecker::finished, this, &MainWindow::finishedThread);
        QObject::connect(worker, &ZipWalkChecker::finishedWData, this, &MainWindow::showInvalidZipInTable);
#ifdef MYPREFIX_DEBUG
        qDebug() << "startThread";
#endif
        thread->start();
    }
}

void MainWindow::showInvalidZipInTable(QSharedPtrListHFIS itemsPtr)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::showInvalidZipInTable";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append(tr("Remove?"));
    horizontalHeader.append(tr("Invalid zip"));
    int columnCount = horizontalHeader.count();
    const int rowCount = itemsPtr.data()->count();
    QTableWidgetItem* tableItem;

    itemsResult = itemsPtr;

    QTableWidget *table = ui->tableWidget;
    QObject::disconnect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::disconnect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);

    QListIterator<HashFileInfoStruct> itemIt(*itemsPtr.data());
    HashFileInfoStruct file;
    int row = 0;
    QString text;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        text = "";
        tableItem = new QTableWidgetItem(text);
        tableItem->setText(text);
        tableItem->setFlags(tableItem->flags() | Qt::ItemIsUserCheckable);
        if(file.checked)
            tableItem->setCheckState(Qt::Checked);
        else
            tableItem->setCheckState(Qt::Unchecked);
        tableItem->setToolTip(file.fileName);
        table->setItem(row, 0, tableItem);

        tableItem = new QTableWidgetItem(file.fileName);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.fileName);
        table->setItem(row, 0, tableItem);
        ++row;
    }

    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
    QObject::connect(table, &QTableWidget::itemChanged, this, &MainWindow::tableWidgetItemChanged);
    QObject::connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::tableWidget_header_clicked);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::closeEvent(QCloseEvent *event) {
#ifdef MYPREFIX_DEBUG
        qDebug() << "MainWindow::closeEvent";
#endif
    if (thread->isRunning()) {
        thread->requestInterruption();
        /*QMessageBox::warning(this,
                             "DirWizard",
                             tr("Background process already running!\nTry to exit after task would be complete."));
        event->ignore();*/
        thread->quit();
        thread->wait(250);
        event->accept();
    } else {
        event->accept();
    }
}

void MainWindow::setUiPushButtonsEnabled(bool flag)
{
    ui->pushButton_Calc_Hashes->setEnabled(flag);
    ui->pushButton_Check_Hashes->setEnabled(flag);
    ui->pushButton_Check_Zip->setEnabled(flag);
    ui->pushButton_Compare_Folders->setEnabled(flag);
    ui->pushButton_Duplicate_Search->setEnabled(flag);
    ui->pushButton_Remove_Checked->setEnabled(flag);
    ui->pushButton_Save_From_Table->setEnabled(flag);
}

void MainWindow::tableWidget_header_clicked(int column) {
    Qt::SortOrder order = ui->tableWidget->horizontalHeader()->sortIndicatorOrder();
    ui->tableWidget->horizontalHeader()->setSortIndicatorShown(true);
#ifdef MYPREFIX_DEBUG
    qDebug() << "MainWindow::tableWidget_header_clicked: " << static_cast<int>(order);
#endif
    ui->tableWidget->sortItems(column, order);
}
