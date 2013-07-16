#include "settingswidget.h"
#include "simulatorapplication.h"

#include <QCheckBox>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QEventLoop>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QSettings>
#include <QTextEdit>
#include <QTextStream>

#define BUTTONTEXTDOWNLOAD "Download"
#define BUTTONTEXTUPDATE "Update"
#define BUTTONTEXTREMOVE "Remove"

void downloadFile(QNetworkAccessManager& manager, const QUrl& url, const QString& filename) {
    QEventLoop loop;
    QNetworkReply* reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    loop.exec();

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());

    delete reply;
}

bool deleteFile(const QString& fileName) {
    if (fileName.size() < 1)
        return true;
    return QFile(fileName).remove();
}

bool deleteDir(const QString& dirName) {
    QDir directory(dirName);

    if (!directory.exists())
        return true;

    QStringList dirs = directory.entryList(QDir::Dirs | QDir::Hidden);
    QStringList files = directory.entryList(QDir::Files | QDir::Hidden);

    QList<QString>::iterator f = files.begin();
    QList<QString>::iterator d = dirs.begin();

    bool error = false;

    for (; f != files.end(); ++f) {
        if (!deleteFile(directory.path() + '/' + (*f)))
            error = true;
    }

    for (; d != dirs.end(); ++d) {
        if ((*d) == "." || (*d) == "..")
            continue;

        if (!deleteDir(directory.path() + '/' + (*d)))
            error = true;
    }

    if (!directory.rmdir(directory.path()))
        error = true;

    return !error;
}

MapEntry::MapEntry(const QString& mapName)
    : mapName_(mapName)
    , nameLabel_(new QLabel)
    , localVersionLabel_(new QLabel)
    , remoteVersionLabel_(new QLabel)
    , actionButton_(new QPushButton)
{}

MapEntry::~MapEntry() {
    delete nameLabel_;
    delete localVersionLabel_;
    delete remoteVersionLabel_;
    delete actionButton_;
}


SettingsWidget::SettingsWidget()
    : QWidget()
    , layout_(0)
    , nameLabel_(0)
    , localVersionLabel_(0)
    , remoteVersionLabel_(0)
    , statusText_(0)
{
    connect(qApp, SIGNAL(remoteVersionFileArrived()), this, SLOT(createMapEntries()));
}

void SettingsWidget::buttonClicked() {
    QObject* sender = QObject::sender();
    foreach (MapEntry* mapEntry, maps_) {
        if (mapEntry->actionButton_ == sender) {
            const QString& mapName = mapEntry->mapName_;
            if (isMapInstalled(mapName)) {
                if (needsUpdate(mapName)) {
                    removeMap(mapName);
                    setButtonsEnabled(false);
                    currentMapDownload_ = mapName;
                    downloadMap(mapName);
                    mapEntry->actionButton_->setText(BUTTONTEXTREMOVE);
                }
                else {
                    removeMap(mapName);
                    emit finishedRemovingMap(mapName);
                    mapEntry->actionButton_->setText(BUTTONTEXTDOWNLOAD);
                }
            }
            else {
                removeMap(mapName);
                setButtonsEnabled(false);
                currentMapDownload_ = mapName;
                downloadMap(mapName);
                mapEntry->actionButton_->setText(BUTTONTEXTREMOVE);
            }
            break;
        }
    }
}

bool SettingsWidget::isMapInstalled(const QString& map) {
    QDir mapsDir = qApp->getMapsDirectory();
    return mapsDir.exists(map);
}

bool SettingsWidget::needsUpdate(const QString& map) {
    QSettings* remote = qApp->getRemoteSettings();
    float remoteVersionFloat = remote->value(map).toFloat();

    float localVersionFloat = qApp->getLocalMapVersion(map).toFloat();

    return (localVersionFloat < remoteVersionFloat);
}

void SettingsWidget::removeMap(const QString& map) {
    QDir mapsDir = qApp->getMapsDirectory();
    deleteDir(mapsDir.absolutePath() + "/" + map);
    mapsDir.remove(map);

    foreach (const MapEntry* entry, maps_) {
        if (entry->mapName_ == map)
            entry->localVersionLabel_->setText("");
    }
}

void SettingsWidget::setupMapEntry(const MapEntry* mapEntry, QSettings* remote) {
    remote->beginGroup("Maps");
    QString mapName = mapEntry->mapName_;
    mapEntry->nameLabel_->setText(mapName);
    QString localVersionString = qApp->getLocalMapVersion(mapName);
    mapEntry->localVersionLabel_->setText(localVersionString);
    const QString& remoteVersionString = remote->value(mapName).toString();
    mapEntry->remoteVersionLabel_->setText(remoteVersionString);
    if (isMapInstalled(mapName)) {
        float localVersionFloat = localVersionString.toFloat();
        float remoteVersionFloat = remote->value(mapName).toFloat();
        if (localVersionFloat < remoteVersionFloat) {
            mapEntry->actionButton_->setText(BUTTONTEXTUPDATE);
        }
        else {
            mapEntry->actionButton_->setText(BUTTONTEXTREMOVE);
        }

    }
    else {
        mapEntry->actionButton_->setText(BUTTONTEXTDOWNLOAD);
    }

    connect(mapEntry->actionButton_, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    remote->endGroup();
}

void SettingsWidget::downloadMap(const QString& map) {
    QDir mapsDir = qApp->getMapsDirectory();
    downloadFile(manager_, qApp->getRemoteMapIndexUrl(map), qApp->getTemporaryIndexFileString());

    QDomDocument doc("document");
    QFile file(qApp->getTemporaryIndexFileString());
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, "File Error", "Could not find the file");
        return;
    }
    QString errorMessage;
    int errorLine;
    if (!doc.setContent(&file, &errorMessage, &errorLine)) {
        QMessageBox::critical(0, "XML Error", errorMessage + "\n" + QString::number(errorLine));
        file.close();
        return;
    }
    file.close();

    mapsDir.mkdir(map);
    mapsDir.cd(map);
    QDomElement docElem = doc.documentElement();
    QDomNodeList factions = docElem.childNodes();
    for (int i = 0; i < factions.size(); ++i) {
        const QDomNode& faction = factions.at(i);
        const QDomElement& factionElem = faction.toElement();
        const QString& factionName = factionElem.nodeName();
        mapsDir.mkdir(factionName);
        mapsDir.cd(factionName);
        QDomNodeList files = factionElem.childNodes();
        for (int j = 0; j < files.size(); ++j) {
            const QDomNode& file = files.at(j);
            const QDomElement& fileElem = file.toElement();
            const QString& fileName = fileElem.nodeName();
            const QString& target = mapsDir.absolutePath() + "/" + fileName + ".png";
            const QString& targetUrl = qApp->getBaseUrlString() + map + "/" + factionName + "/" + fileName + ".png";
            QNetworkReply* reply = manager_.get(QNetworkRequest(QUrl(targetUrl)));
            replyFileNameMap_.insert(reply, target);
            connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        }
        mapsDir.cdUp();
    }
    QNetworkReply* reply = manager_.get(QNetworkRequest(QUrl(qApp->getBaseUrlString() + map + "/" + map + ".xml")));
    replyFileNameMap_.insert(reply, mapsDir.absolutePath() + "/" + map + ".xml");
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));

    QSettings* remote = qApp->getRemoteSettings();
    remote->beginGroup("Maps");
    QFile versionFile(qApp->getLocalMapVersionFileString(map));
    versionFile.open(QIODevice::WriteOnly);
    versionFile.write(remote->value(map).toString().toLocal8Bit());
    remote->endGroup();
}

void SettingsWidget::downloadFinished() {
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(QObject::sender());

    const QString& filename = replyFileNameMap_.take(reply);

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    statusText_->append("Downloaded " + filename);

    if (replyFileNameMap_.keys().isEmpty()) {
        createMapEntries();
        emit finishedDownloadingMap(currentMapDownload_);
         setButtonsEnabled(true);
    }
}

void SettingsWidget::createMapEntries() {
    foreach (const MapEntry* mapEntry, maps_)
        delete mapEntry;
    maps_.clear();

    QSettings* remoteSettings = qApp->getRemoteSettings();

    remoteSettings->beginGroup("Maps");
    QStringList allMaps = remoteSettings->allKeys();

    if (!layout_)
        layout_ = new QGridLayout(this);

    if (!nameLabel_) {
        nameLabel_ = new QLabel("Name");
        layout_->addWidget(nameLabel_, 0, 0);
    }

    if (!localVersionLabel_) {
        localVersionLabel_ = new QLabel("Local Version");
        layout_->addWidget(localVersionLabel_, 0, 1);
    }

    if (!remoteVersionLabel_) {
        remoteVersionLabel_ = new QLabel("Remote Version");
        layout_->addWidget(remoteVersionLabel_, 0, 2);
    }

    int i = 0;
    remoteSettings->endGroup();
    for (; i < allMaps.size(); ++i) {
        const QString& map = allMaps[i];
        MapEntry* mapEntry = new MapEntry(map);
        setupMapEntry(mapEntry, remoteSettings);
        maps_.append(mapEntry);
        layout_->addWidget(mapEntry->nameLabel_, i+1, 0);
        layout_->addWidget(mapEntry->localVersionLabel_, i+1, 1);
        layout_->addWidget(mapEntry->remoteVersionLabel_, i+1, 2);
        layout_->addWidget(mapEntry->actionButton_, i+1, 3);
    }


    if (!statusText_) {
        statusText_ = new QTextEdit;
        statusText_->setReadOnly(true);
        layout_->addWidget(statusText_, i+1, 0, 3, 0);
        layout_->setRowStretch(i+1, 1);
    }
}

void SettingsWidget::setButtonsEnabled(bool enabled) {
    foreach (MapEntry* map, maps_) {
        map->actionButton_->setEnabled(enabled);
    }
}
