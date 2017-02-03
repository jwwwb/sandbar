//
// Created by jwwwb on 2016-10-17.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QObject>
#include <QTime>
#include <iomanip> // setprecision
#include <sstream> // stringstream

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new Model();
    ui->statusBar->addWidget(ui->statusLabel);
    currentPlaylist = -1;
    playback = new PlaybackController();
    initalizePlaylistTable();
    QCoreApplication::setOrganizationName("jwwwb");
    QCoreApplication::setOrganizationDomain("jwwwb.com");
    QCoreApplication::setApplicationName("Sandbar");
    connectSignals();
    ui->playlistTabs->setUsesScrollButtons(true);
    if (ui->playlistTabs->count() == 0) { newPlaylist(); }

    QSettings settings;
    int volume = settings.value("volume", 256).toInt();
    ui->volumeSlider->setValue(volume);
    // update the label value:
    on_volumeSlider_sliderMoved(volume);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete playback;
    delete model;
}

// private methods

void MainWindow::connectSignals()
{
    // ui to model
    connect(this, SIGNAL(signalNextPushed()), model, SLOT(requestNextFile()));
    connect(this, SIGNAL(signalPreviousPushed()), model, SLOT(requestPreviousFile()));

    // model to playback
    connect(model, SIGNAL(signalCurrentFile(QString)), playback->decoder, SLOT(slotFile(QString)));
    connect(model, SIGNAL(signalNoMoreFiles()), playback->decoder, SLOT(slotStop()));

    // playback to model
    connect(playback->decoder, SIGNAL(signalRequestFile()), model, SLOT(requestCurrentFile()));
    connect(playback->decoder, SIGNAL(signalFileEnded()), model, SLOT(requestNextFile()));

    // status updates
    connect(playback->decoder, SIGNAL(signalDuration(qlonglong)), this, SLOT(slotDuration(qlonglong)));
    connect(playback->decoder, SIGNAL(signalPlaybackProgress(qlonglong)), this, SLOT(slotPlaybackProgress(qlonglong)));
    connect(playback->decoder, SIGNAL(signalFileEnded()), this, SLOT(slotPlaybackEnded()));

    // TODO connect finishedPlaying to something that kills the progressSlider
}

void MainWindow::deletePlaylist(int index)
{
    model->removePlaylist(index);
    listOfPlaylistIDs.erase(listOfPlaylistIDs.begin()+index);
    listOfPlaylistNames.erase(listOfPlaylistNames.begin()+index);
    listOfPlaylistPages.erase(listOfPlaylistPages.begin()+index);
    currentPlaylist = model->getInFocusPlaylistIndex();
}

QList<int> MainWindow::getSelectedInPlaylist()
{
    QList<int> selecteds;
    QItemSelectionModel *select = playlistTableWidget->selectionModel();
    if (select->hasSelection()) {
        for (int i = 0; i < select->selectedRows().size(); ++i) {
            selecteds << select->selectedRows().at(i).row();
        }
    }
    return selecteds;
}

void MainWindow::initalizePlaylistTable()
{
    QStringList columnHeaders;
    QSettings settings;
//    settings.clear();
    qDebug() << "initializing table";
    if (settings.contains("column_headers")) {
        columnHeaders = settings.value("column_headers").toStringList();
    } else {
        columnHeaders << "Artist Name" << "Track Title" << "Album Title" << "Duration" << "File Path";
        settings.setValue("column_headers", columnHeaders);
    }
    QList<QVariant> columnWidths;
    if (settings.contains("column_widths")) {
        columnWidths = settings.value("column_widths").toList();
        qDebug() << "column widths settings already exists";
    } else {
        columnWidths = {30, 180, 140, 130, 90, 250};
        settings.setValue("column_widths", columnWidths);
        qDebug() << "storing column widths settings";
    }
    columnHeaders.prepend("Playing");
    playlistTableWidget = new QTableWidget(0, columnHeaders.size());
    int columnIndex = 0;
    for (QVariant width : columnWidths) {
        playlistTableWidget->setColumnWidth(columnIndex, width.toInt());
        ++columnIndex;
    }
    playlistTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    playlistTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    playlistTableWidget->setHorizontalHeaderLabels(columnHeaders);
    playlistTableWidget->setAcceptDrops(true);
    connect(playlistTableWidget->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(slotColumnResized(int,int,int)));
    connect(playlistTableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(rightClickPlaylist(const QPoint&)));
    connect(playlistTableWidget, SIGNAL(cellDoubleClicked(int,int)), model, SLOT(jumpToFile(int)));
}

void MainWindow::newPlaylist()
{
    QWidget *page = new QWidget();
    QGridLayout *layout2 = new QGridLayout;
    page->setLayout(layout2);
    QString newListName = model->newPlaylist();
    ui->playlistTabs->addTab(page, newListName);
    currentPlaylist = ui->playlistTabs->count()-1;
    ui->playlistTabs->setCurrentIndex(currentPlaylist);
    listOfPlaylistIDs.push_back(ui->playlistTabs->count()-1);
    listOfPlaylistNames.push_back(newListName);
    listOfPlaylistPages.push_back(page);
    page->layout()->addWidget(playlistTableWidget);
    playlistTableWidget->setRowCount(0);
    playlistTableWidget->clearContents();
}

void MainWindow::updatePlaylist()
{
    QStringList columnHeaders;
    QSettings settings;
    if (settings.contains("column_headers")) {
        columnHeaders = settings.value("column_headers").toStringList();
    } else {
        columnHeaders << "Artist Name" << "Track Title" << "Album Title" << "Duration" << "File Path";
        settings.setValue("column_headers", columnHeaders);
    }
    if (ui->playlistTabs->count() <= currentPlaylist) { return; }
    ui->playlistTabs->setCurrentIndex(currentPlaylist);
    QTableWidgetItem *item = NULL;
    if (!model->getInFocusPlaylist().empty()) {
        qDebug() << "update playlist is trying to populate the tablewidget";
        playlistTableWidget->setRowCount(model->getInFocusPlaylist().size());
        int rowIndex = 0;
        for (MediaFile *media : model->getInFocusPlaylist().mediaFiles) {
            QTableWidgetItem *i = new QTableWidgetItem("");
            i->setFlags(i->flags() &  ~Qt::ItemIsEditable);
            playlistTableWidget->setItem(rowIndex, 0, i);
            int columnIndex = 1; // start at 1, because 0 is "Playing"
            for (QString key : columnHeaders) {
                if (media->allData.contains(key)) {
                    item = new QTableWidgetItem(media->allData.value(key).st);
                } else {
                    item = new QTableWidgetItem("");
                }
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
                playlistTableWidget->setItem(rowIndex, columnIndex, item);
                columnIndex++;
            }
            rowIndex++;
        }
        qDebug() << "all files added";
    } else {
        playlistTableWidget->clearContents();
        playlistTableWidget->setRowCount(0);
    }
}

// public slots

void MainWindow::playPlaylistEntry()
{
    model->jumpToFile(getSelectedInPlaylist().at(0));
    qDebug() << "Requested Playback of" << getSelectedInPlaylist() << "items";
}

void MainWindow::rightClickPlaylist(const QPoint & pos)
{
    QPoint globalPos = playlistTableWidget->viewport()->mapToGlobal(pos);
    QMenu rightClickMenu;
    // TODO adjust the actions based on which songs are selected

    QList<int> selected = getSelectedInPlaylist();
    if (selected.size() == 1) {
        rightClickMenu.addAction(QString("Play"), this, SLOT(playPlaylistEntry()));
        rightClickMenu.addAction("Delete");
        rightClickMenu.addAction("Properties");
    } else {
        rightClickMenu.addAction(QString("Delete %1 Files").arg(selected.size()));
        rightClickMenu.addAction("Properties");
    }
    QAction* selectedItem = rightClickMenu.exec(globalPos);
    // I might not need this section at all:
    if (selectedItem)
    {
        qDebug() << "right click menu selected item:" << selectedItem;
    }
    else
    {
        qDebug() << "no option chosen in right click menu.";
    }
}

void MainWindow::slotColumnResized(int idx, int oldsize, int newsize)
{
    QSettings settings;
    QList<QVariant> columnWidths;
    if (settings.contains("column_widths")) {
        columnWidths = settings.value("column_widths").toList();
//        qDebug() << "column widths settings already exists";
    } else {
        columnWidths = {30, 180, 140, 130, 90, 250};
        settings.setValue("column_widths", columnWidths);
//        qDebug() << "storing column widths settings";
    }
//    qDebug() << "resizing column, parameters: idx =" << idx << ",old =" << oldsize << ",new =" << newsize;
    columnWidths = settings.value("column_widths").toList();
//    qDebug() << "column widths:" << columnWidths;
//    qDebug() << "columnWidths[idx]" << columnWidths[idx];
    columnWidths[idx] = (qlonglong)newsize;
//    qDebug() << "column widths:" << columnWidths;
    settings.setValue("column_widths", columnWidths);
    settings.sync();
}

void MainWindow::slotDuration(qlonglong duration)
{
    qDebug() << "received new duration of" << duration;
    ui->progressSlider->setMaximum((int)(duration*uSecToSliderStep)-1);
    ui->progressSlider->setMinimum(0);
}

void MainWindow::slotPlaybackEnded()
{
    if (!freezeUpdates) {
        ui->progressSlider->setValue(ui->progressSlider->maximum());
    }
}

void MainWindow::slotPlaybackProgress(qlonglong timePlayed)
{
    if (!freezeUpdates) {
        ui->progressSlider->setValue((int)(timePlayed*uSecToSliderStep));
    }
//    qDebug() << "received new progress of" << timePlayed;
    qlonglong phours = (qlonglong)floor((double)timePlayed/(3600.0 * AV_TIME_BASE));
    int pmins = (int)floor((timePlayed-AV_TIME_BASE*phours*3600)/(60*AV_TIME_BASE));
    int psecs = (int)floor((timePlayed-AV_TIME_BASE*(phours*3600+pmins*60))/AV_TIME_BASE);

    qlonglong dur = playback->getDuration();
    qlonglong dhours = (qlonglong)floor((double)dur/(3600.0 * AV_TIME_BASE));
    int dmins = (int)floor((dur-AV_TIME_BASE*dhours*3600)/(60*AV_TIME_BASE));
    int dsecs = (int)floor((dur-AV_TIME_BASE*(dhours*3600+dmins*60))/AV_TIME_BASE);

    QString status = QString("%0:%1:%2 / %3:%4:%5").arg(phours).arg(pmins, 2, 10, QChar('0')).arg(psecs, 2, 10, QChar('0')).arg(dhours).arg(dmins, 2, 10, QChar('0')).arg(dsecs, 2, 10, QChar('0'));
    ui->statusLabel->setText(status);
}

// slots

void MainWindow::on_actionAdd_File_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add File(s)", QString(getenv("HOME"))+"/Music", "Audio Files (*.mp3 *.flac *.ogg *.wav)");
    if (!fileNames.isEmpty()) {
        for (QString fileName : fileNames) {
            qDebug() << "trying to create MediaFile with filename:" << fileName;
            MediaFile *mediaFile = new MediaFile(fileName);
            qDebug() << "made it, adding to playlist";
            model->addToPlaylist(mediaFile);
            qDebug() << "added";
        }
        this->updatePlaylist();
    }
}

void MainWindow::on_actionNew_Playlist_triggered()
{
    newPlaylist();
}

// TODO let this guy handle folders as well
void MainWindow::on_actionOpen_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open File(s)", QString(getenv("HOME"))+"/Music", "Audio Files (*.mp3 *.flac *.ogg *.wav)");
    if (!fileNames.isEmpty()) {
        this->model->clearPlaylist();
        for (QString fileName : fileNames) {
            MediaFile *mediaFile = new MediaFile(fileName);
            model->addToPlaylist(mediaFile);
        }
        this->updatePlaylist();
    }
}

void MainWindow::on_playlistTabs_currentChanged(int index)
{
    currentPlaylist = index;
    model->switchPlaylist(index);
    // only do this when switching to an existing playlist, not when creating a new one.
    if (listOfPlaylistPages.size() > index) {
        qDebug() << "attempting to change playlist to" << index << "of" << listOfPlaylistPages.size();
        listOfPlaylistPages.at(index)->layout()->addWidget(playlistTableWidget);
        updatePlaylist();
    }
}

void MainWindow::on_playlistTabs_tabBarDoubleClicked(int index)
{
    if (index < 0) {
        newPlaylist();
    } else {
        // todo let user specify name
        QString newName = "renamed";
        model->renamePlaylist(newName);
        ui->playlistTabs->setTabText(index, newName);
    }
}

void MainWindow::on_playlistTabs_tabCloseRequested(int index)
{
    deletePlaylist(index);
    ui->playlistTabs->removeTab(index);
}

void MainWindow::on_progressSlider_sliderPressed()
{
//    qDebug() << "slider pressed";
    freezeUpdates = 1;
}

void MainWindow::on_progressSlider_sliderReleased()
{
    int val = ui->progressSlider->value();
    int pos = ui->progressSlider->sliderPosition();
    qDebug() << "slider released at value" << val << "and position" << pos;
    freezeUpdates = 0;
    playback->seekFile(val*sliderStepToUSec);
}

void MainWindow::on_pushButtonClear_clicked()
{
    model->clearPlaylist();
    this->updatePlaylist();
}

void MainWindow::on_pushButtonHello_clicked()
{
    qDebug() << "trying to create MediaFile from mannfred mann";
    MediaFile *mediaFile = new MediaFile("/Users/yames/Music/02. Manfred Mann's Earth Band - Blinded By The Light.flac");
    qDebug() << "made it, adding to playlist";
    model->addToPlaylist(mediaFile);
    qDebug() << "added, calling update playlist";
    this->updatePlaylist();
    qDebug() << "call to update returned.";
}

void MainWindow::on_pushButtonNext_clicked()
{
    emit signalNextPushed();
//    playback->seekDifferential(5*sample_rate);
}

void MainWindow::on_pushButtonPause_clicked()
{
    playback->pausePushed();
}

void MainWindow::on_pushButtonPlay_clicked()
{
    playback->playPushed();
}

void MainWindow::on_pushButtonPrevious_clicked()
{
    emit signalPreviousPushed();
//    playback->seekDifferential(-5*sample_rate);
}

void MainWindow::on_pushButtonRandom_clicked()
{
    // TODO change this to something with random.
    qDebug() << "pushed seek button";
    model->requestRandomFile();
//    playback->seekFile(5000000);
}

void MainWindow::on_pushButtonStop_clicked()
{
    playback->stopPushed();
}

void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    float volumeMultiplier = 0;
    float dBLevel = 0;
    if (position == 0) {
        ui->volumeLabel->setText("-∞dB");
    } else {
        dBLevel = 10*log2(position/256.0);
        volumeMultiplier = pow(10.0, dBLevel/10.0);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << dBLevel;
        std::string dBString = ss.str();
        // TODO Q-ify
        ui->volumeLabel->setText((dBString + "dB").c_str());
    }
    playback->setVolume(volumeMultiplier);
    QSettings settings;
    settings.setValue("volume", position);
}
