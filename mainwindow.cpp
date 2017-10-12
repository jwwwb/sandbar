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
    playlistInFocus = -1;
    playback = new PlaybackController();

    QCoreApplication::setOrganizationName("jwwwb");
    QCoreApplication::setOrganizationDomain("jwwwb.com");
    QCoreApplication::setApplicationName("Sandbar");
    connectSignals();
    ui->playlistTabs->setUsesScrollButtons(true);
    initalizePlaylistTable();
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
    connect(this, SIGNAL(signalNextPushed(bool)), model, SLOT(requestNextFile()));
    connect(this, SIGNAL(signalPreviousPushed()), model, SLOT(requestPreviousFile()));

    // model to playback
    connect(model, SIGNAL(signalCurrentFile(QString)), playback->decoder, SLOT(slotFile(QString)));
    connect(model, SIGNAL(signalCurrentFileSoon(QString)), playback->decoder, SLOT(slotFileSoon(QString)));
    connect(model, SIGNAL(signalNoMoreFiles()), playback->decoder, SLOT(slotStop()));

    // playback to model
    connect(playback->decoder, SIGNAL(signalRequestFile()), model, SLOT(requestCurrentFile()));
    connect(playback->decoder, SIGNAL(signalFileReadEnded()), model, SLOT(requestNextFileSoon()));

    // status updates
    connect(playback->decoder, SIGNAL(signalDuration(qlonglong)), this, SLOT(slotDuration(qlonglong)));
    connect(playback->decoder, SIGNAL(signalPlaybackProgress(qlonglong)), this, SLOT(slotPlaybackProgress(qlonglong)));
//    connect(playback->decoder, SIGNAL(signalFileEnded()), this, SLOT(slotPlaybackEnded()));      // calling this now is too early


    // playlist table widget
    // TODO move first two to ui designer
    connect(ui->playlistTableWidget->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(slotColumnResized(int,int,int)));
    connect(ui->playlistTableWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(rightClickPlaylist(const QPoint&)));
    connect(ui->playlistTableWidget, SIGNAL(cellDoubleClicked(int,int)), model, SLOT(jumpToFile(int)));

    // TODO connect finishedPlaying to something that kills the progressSlider
}

void MainWindow::deletePlaylist(int index)
{
    model->removePlaylist(index);
    listOfPlaylistIDs.erase(listOfPlaylistIDs.begin()+index);
    listOfPlaylistNames.erase(listOfPlaylistNames.begin()+index);
    playlistInFocus = model->getInFocusPlaylistIndex();
}

QList<int> MainWindow::getSelectedInPlaylist()
{
    QList<int> selecteds;
    QItemSelectionModel *select = ui->playlistTableWidget->selectionModel();
    if (select->hasSelection()) {
        for (int i = 0; i < select->selectedRows().size(); ++i) {
            selecteds << select->selectedRows().at(i).row();
        }
    }
    return selecteds;
}

void MainWindow::initalizePlaylistTable()
{
    // TODO some of this can now be moved to the UI designer.
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
    ui->playlistTableWidget->setColumnCount(columnHeaders.size());
    int columnIndex = 0;
    for (QVariant width : columnWidths) {
        ui->playlistTableWidget->setColumnWidth(columnIndex, width.toInt());
        ++columnIndex;
    }
    ui->playlistTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->playlistTableWidget->setHorizontalHeaderLabels(columnHeaders);
    ui->playlistTableWidget->setAcceptDrops(true);
}

void MainWindow::newPlaylist()
{
    QString newListName = model->newPlaylist();
    ui->playlistTabs->addTab(new QWidget(), newListName);
    playlistInFocus = ui->playlistTabs->count()-1;
    ui->playlistTabs->setCurrentIndex(playlistInFocus);
    listOfPlaylistIDs.push_back(ui->playlistTabs->count()-1);
    listOfPlaylistNames.push_back(newListName);
    ui->playlistTableWidget->setRowCount(0);
    ui->playlistTableWidget->clearContents();
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
    if (ui->playlistTabs->count() <= playlistInFocus) { return; }
    ui->playlistTabs->setCurrentIndex(playlistInFocus);
    QTableWidgetItem *item = NULL;
    if (!model->getInFocusPlaylist().empty()) {
        ui->playlistTableWidget->setRowCount(model->getInFocusPlaylist().size());
        int rowIndex = 0;
        for (MediaFile *media : model->getInFocusPlaylist().mediaFiles) {
            QTableWidgetItem *i = new QTableWidgetItem("");
            i->setFlags(i->flags() &  ~Qt::ItemIsEditable);
            ui->playlistTableWidget->setItem(rowIndex, 0, i);
            int columnIndex = 1; // start at 1, because 0 is "Playing"
            for (QString key : columnHeaders) {
                if (media->allData.contains(key)) {
                    item = new QTableWidgetItem(media->allData.value(key).st);
                } else {
                    item = new QTableWidgetItem("");
                }
                item->setFlags(item->flags() &  ~Qt::ItemIsEditable);
                ui->playlistTableWidget->setItem(rowIndex, columnIndex, item);
                columnIndex++;
            }
            rowIndex++;
        }
    } else {
        ui->playlistTableWidget->clearContents();
        ui->playlistTableWidget->setRowCount(0);
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
    QPoint globalPos = ui->playlistTableWidget->viewport()->mapToGlobal(pos);
    QMenu rightClickMenu;
    // TODO adjust the actions based on which songs are selected

    QList<int> selected = getSelectedInPlaylist();
    if (selected.size() == 1) {
        rightClickMenu.addAction(QString("Play"), this, SLOT(playPlaylistEntry()));
        rightClickMenu.addSeparator();
        rightClickMenu.addAction("Remove");
        rightClickMenu.addAction("Crop");
    } else {
        rightClickMenu.addAction(QString("Remove %1 Files").arg(selected.size()));
        rightClickMenu.addAction(QString("Crop to %1 Files").arg(selected.size()));
    }
    rightClickMenu.addSeparator();
    rightClickMenu.addAction("Cut");
    rightClickMenu.addAction("Copy");
    rightClickMenu.addAction("Paste");
    rightClickMenu.addSeparator();
    rightClickMenu.addAction("Properties");
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
    qlonglong phours = (qlonglong)floor((double)timePlayed/(3600.0 * timeBase));
    int pmins = (int)floor((timePlayed-timeBase*phours*3600)/(60*timeBase));
    int psecs = (int)floor((timePlayed-timeBase*(phours*3600+pmins*60))/timeBase);

    qlonglong dur = playback->getDuration();
    qlonglong dhours = (qlonglong)floor((double)dur/(3600.0 * timeBase));
    int dmins = (int)floor((dur-timeBase*dhours*3600)/(60*timeBase));
    int dsecs = (int)floor((dur-timeBase*(dhours*3600+dmins*60))/timeBase);

    QString status = QString("%0:%1:%2 / %3:%4:%5").arg(phours).arg(pmins, 2, 10, QChar('0')).arg(psecs, 2, 10, QChar('0')).arg(dhours).arg(dmins, 2, 10, QChar('0')).arg(dsecs, 2, 10, QChar('0'));
    ui->statusLabel->setText(status);
}

// slots

void MainWindow::on_actionAdd_File_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Add File(s)", QString(getenv("HOME"))+"/Music", "Audio Files (*.mp3 *.flac *.ogg *.wav)");
    if (!fileNames.isEmpty()) {
        for (QString fileName : fileNames) {
            MediaFile *mediaFile = new MediaFile(fileName);
            model->addToPlaylist(mediaFile);
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
    playlistInFocus = index;
    model->switchPlaylist(index);
    updatePlaylist();
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
    // TODO: prevent program from crashing when closing last tab.
    deletePlaylist(index);
    ui->playlistTabs->removeTab(index);
}

void MainWindow::on_progressSlider_sliderPressed()
{
    freezeUpdates = 1;
}

void MainWindow::on_progressSlider_sliderReleased()
{
    int val = ui->progressSlider->value();
    int pos = ui->progressSlider->sliderPosition();
//    qDebug() << "slider released at value" << val << "and position" << pos;
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
    MediaFile *mediaFile = new MediaFile("/Users/yames/Music/02. Manfred Mann's Earth Band - Blinded By The Light.flac");
    model->addToPlaylist(mediaFile);
    this->updatePlaylist();
}

void MainWindow::on_pushButtonNext_clicked()
{
//    emit signalNextPushed();
    playback->seekDifferential(5*timeBase);
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
//    emit signalPreviousPushed();
    playback->seekDifferential(-5*timeBase);
}

void MainWindow::on_pushButtonRandom_clicked()
{
    model->requestRandomFile();
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
