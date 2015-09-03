#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dirNameForFolderDialog(QDir::current().dirName()),
    itemsB(nullptr)
{
    ui->setupUi(this);
    initDirsListWidget();
    initHashComboBoxWidget();
    initTableWidget();
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::on_AboutAction_Triggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Dirs List Widget Start
void MainWindow::initDirsListWidget()
{
   ui->listWidget->clear();
   // QObject::connect here in the Past
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
    for(int i=0; i < count; i++)
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
    //QObject::connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(currentComboBoxIndexChanged(int)));
    //QObject::connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::currentComboBoxIndexChanged);
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
    horizontalHeader.append("Remove?");
    horizontalHeader.append("groupId");
    horizontalHeader.append("Hash");
    horizontalHeader.append("Size");
    horizontalHeader.append("FileName");
    int columnCount = horizontalHeader.count();
    int rowCount = 0;
    QTableWidget *table = ui->tableWidget;
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);
}

void MainWindow::showDuplicatesInTable(QList<HashFileInfoStruct> *items)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "showDuplicatesInTable";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append("Remove?");
    horizontalHeader.append("groupId");
    horizontalHeader.append("Hash");
    horizontalHeader.append("Size");
    horizontalHeader.append("FileName");
    int columnCount = horizontalHeader.count();
    int rowCount = items->count();
    QTableWidgetItem* tableItem;

    QTableWidget *table = ui->tableWidget;
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);

    QListIterator<HashFileInfoStruct> itemIt(*items);
    HashFileInfoStruct file;
    int row = 0;
    QString text;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        text = QString("");
        tableItem = new QTableWidgetItem(text);
        tableItem->setText(text);
        tableItem->setFlags(tableItem->flags() | Qt::ItemIsUserCheckable);
        tableItem->setCheckState(Qt::Unchecked);
        tableItem->setData(Qt::UserRole, QVariant(file.fileName));
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
        row++;
    }

    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
    itemsB = items;
}

QList<QString> MainWindow::getCheckedFileNamesFormTable()
{
    qDebug() << "getCheckedFileNamesFormTable";
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
// END

// Duplicate Files Search
void MainWindow::startDuplicateSearchInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startDuplicateSearchInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty())
    {
        QThread* thread = new QThread;
        DuplicateFinder *worker = new DuplicateFinder(getCurrentHashAlgo(), 0);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        //connect(thread, SIGNAL(started()), worker, SLOT(process()));
        QObject::connect(thread, &QThread::started, worker, &DuplicateFinder::process);
        //connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, &DuplicateFinder::finished, thread, &QThread::quit);
        QObject::connect(worker, &DuplicateFinder::finished, this, &MainWindow::finishedThread);
        //connect(worker, SIGNAL(finishedWData(QList<HashFileInfoStruct> *)), this, SLOT(showDuplicatesInTable(QList<HashFileInfoStruct> *)));
        QObject::connect(worker, &DuplicateFinder::finishedWData, this, &MainWindow::showDuplicatesInTable);
        QObject::connect(this, &MainWindow::stopThread, worker, &DuplicateFinder::stop);
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
void MainWindow::compareFoldersComplete(QList<HashFileInfoStruct> *items)
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "compareFoldersComplete";
#endif
    QStringList horizontalHeader;
    horizontalHeader.append("Remove?");
    horizontalHeader.append("Hash");
    horizontalHeader.append("Size");
    horizontalHeader.append("FileName");
    int columnCount = horizontalHeader.count();
    int rowCount = items->count();
    QTableWidgetItem* tableItem;

    QTableWidget *table = ui->tableWidget;
    table->clearContents();
    table->setColumnCount(columnCount);
    table->setRowCount(rowCount);
    table->setHorizontalHeaderLabels(horizontalHeader);

    QListIterator<HashFileInfoStruct> itemIt(*items);
    HashFileInfoStruct file;
    int row = 0;
    QString text;
    while(itemIt.hasNext())
    {
        file = itemIt.next();

        text = QString("");
        tableItem = new QTableWidgetItem(text);
        tableItem->setFlags(tableItem->flags() | Qt::ItemIsUserCheckable);
        tableItem->setCheckState(Qt::Unchecked);
        tableItem->setData(Qt::UserRole, QVariant(file.fileName));
        tableItem->setToolTip(file.fileName);
        tableItem->setText(text);
        table->setItem(row, 0, tableItem);


        tableItem = new QTableWidgetItem(file.hash);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.hash);
        table->setItem(row, 1, tableItem);

        text = QString("%1").arg(file.size);
        tableItem = new QTableWidgetItem(text);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(text);
        table->setItem(row, 2, tableItem);

        tableItem = new QTableWidgetItem(file.fileName);
        tableItem->setToolTip(file.fileName);
        tableItem->setText(file.fileName);
        table->setItem(row, 3, tableItem);
        row++;
    }

    QHeaderView* header = table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    table->resizeColumnsToContents();
}

void MainWindow::startComparingFoldersInBackground()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "startComparingFoldersInBackground";
#endif
    QList<QDir> dirs = getElementsFromDirsListWidget();
    if(!dirs.isEmpty())
    {
        QThread* thread = new QThread;
        DirComparator *worker = new DirComparator(getCurrentHashAlgo(), 0);

        worker->setQDir(dirs);
        worker->moveToThread(thread);
        //connect(thread, SIGNAL(started()), worker, SLOT(process()));
        QObject::connect(thread, &QThread::started, worker, &DirComparator::process);
        //connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        QObject::connect(worker, &DirComparator::finished, thread, &QThread::quit);
        QObject::connect(worker, &DirComparator::finished, this, &MainWindow::finishedThread);
        //connect(worker, SIGNAL(finishedWData(QList<HashFileInfoStruct> *)), this, SLOT(compareFoldersComplete(QList<HashFileInfoStruct> *)));
        QObject::connect(worker, &DirComparator::finishedWData, this, &MainWindow::compareFoldersComplete);
        QObject::connect(this, &MainWindow::stopThread, worker, &DirComparator::stop);
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
    QList<QString> dirs = getCheckedFileNamesFormTable();
    if(!dirs.isEmpty())
    {
        QString dir;
        QListIterator<QString> it(dirs);
        dir = it.next();
        QDir(dir).remove(dir);
    }
}

/*
 * Slot for Start Duplicate file Search in background (other function)
 */
void MainWindow::on_pushButton_Duplicate_Search_clicked()
{
#ifdef MYPREFIX_DEBUG
    qDebug() << "on_pushButton_Duplicate_Search_clicked";
#endif
    startDuplicateSearchInBackground();
}

/*
 * Remove Seleccted item (Dir Name) from ListWidget
 */
void MainWindow::on_pushButton_RemoveDir_clicked()
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
    emit stopThread();
}

void MainWindow::on_AboutAction_Triggered(bool checked)
{
    QMessageBox::about(this,
                       QString("About DirWizard"),
                       QString("Written by Yuriy (Yuri) Astrov\nBased on QT 5."));

 }

void MainWindow::finishedThread()
{
    QMessageBox::information(this,
                             QString("DirWizard"),
                             QString("Task completed successfully!"));
}


void MainWindow::on_pushButton_Compare_Folders_clicked()
{
    startComparingFoldersInBackground();
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
        QString delimeter = QString("\t");
        QString endOfLine = QString("\n");

        out << QString("%Group ID") << delimeter;
        out << QString("Size") << delimeter;
        out << QString("Hash") << delimeter << QString("FileName");
        out << endOfLine;

        QListIterator<HashFileInfoStruct> it(*itemsB);
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
    if(itemsB != nullptr)
    {
    QString fileName = QFileDialog::getSaveFileName(this, QString("Save File"),
                                     QDir::homePath(),
                                     QString("Text files (*.txt)") );
    if(!fileName.isNull() && !fileName.isEmpty())
    {
        QFileInfo qF(fileName);
        if(qF.exists())
        {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "File already exists!", "Overwrite?",
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
