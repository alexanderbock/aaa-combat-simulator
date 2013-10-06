/**************************************************************************************************
 *                                                                                                *
 * AAA Combat Simulator                                                                           *
 *                                                                                                *
 * Copyright (c) 2011 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *                                                                                                *
 *************************************************************************************************/

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
    : mapName(mapName)
    , nameLabel(new QLabel)
    , localVersionLabel(new QLabel)
    , remoteVersionLabel(new QLabel)
    , actionButton(new QPushButton)
{}

MapEntry::~MapEntry() {
    delete nameLabel;
    delete localVersionLabel;
    delete remoteVersionLabel;
    delete actionButton;
}


SettingsWidget::SettingsWidget()
    : QWidget()
    , _layout(0)
    , _nameLabel(0)
    , _localVersionLabel(0)
    , _remoteVersionLabel(0)
    , _statusText(0)
{
    connect(qApp, SIGNAL(remoteVersionFileArrived()), this, SLOT(createMapEntries()));
}

void SettingsWidget::buttonClicked() {
    QObject* sender = QObject::sender();
    foreach (MapEntry* mapEntry, _maps) {
        if (mapEntry->actionButton == sender) {
            const QString& mapName = mapEntry->mapName;
            if (isMapInstalled(mapName)) {
                if (needsUpdate(mapName)) {
                    removeMap(mapName);
                    setButtonsEnabled(false);
                    _currentMapDownload = mapName;
                    downloadMap(mapName);
                    mapEntry->actionButton->setText(BUTTONTEXTREMOVE);
                }
                else {
                    removeMap(mapName);
                    emit finishedRemovingMap(mapName);
                    mapEntry->actionButton->setText(BUTTONTEXTDOWNLOAD);
                }
            }
            else {
                removeMap(mapName);
                setButtonsEnabled(false);
                _currentMapDownload = mapName;
                downloadMap(mapName);
                mapEntry->actionButton->setText(BUTTONTEXTREMOVE);
            }
            break;
        }
    }
}

bool SettingsWidget::isMapInstalled(const QString& map) {
    QDir mapsDir = qApp->mapsDirectory();
    return mapsDir.exists(map);
}

bool SettingsWidget::needsUpdate(const QString& map) {
    QSettings* remote = qApp->remoteSettings();
    float remoteVersionFloat = remote->value(map).toFloat();

    float localVersionFloat = qApp->localMapVersion(map).toFloat();

    return (localVersionFloat < remoteVersionFloat);
}

void SettingsWidget::removeMap(const QString& map) {
    QDir mapsDir = qApp->mapsDirectory();
    deleteDir(mapsDir.absolutePath() + "/" + map);
    mapsDir.remove(map);

    foreach (const MapEntry* entry, _maps) {
        if (entry->mapName == map)
            entry->localVersionLabel->setText("");
    }
}

void SettingsWidget::setupMapEntry(const MapEntry* mapEntry, QSettings* remote) {
    remote->beginGroup("Maps");
    QString mapName = mapEntry->mapName;
    mapEntry->nameLabel->setText(mapName);
    QString localVersionString = qApp->localMapVersion(mapName);
    mapEntry->localVersionLabel->setText(localVersionString);
    const QString& remoteVersionString = remote->value(mapName).toString();
    mapEntry->remoteVersionLabel->setText(remoteVersionString);
    if (isMapInstalled(mapName)) {
        float localVersionFloat = localVersionString.toFloat();
        float remoteVersionFloat = remote->value(mapName).toFloat();
        if (localVersionFloat < remoteVersionFloat) {
            mapEntry->actionButton->setText(BUTTONTEXTUPDATE);
        }
        else {
            mapEntry->actionButton->setText(BUTTONTEXTREMOVE);
        }

    }
    else {
        mapEntry->actionButton->setText(BUTTONTEXTDOWNLOAD);
    }

    connect(mapEntry->actionButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    remote->endGroup();
}

void SettingsWidget::downloadMap(const QString& map) {
    QDir mapsDir = qApp->mapsDirectory();
    downloadFile(_manager, qApp->remoteMapIndexURL(map), qApp->temporaryIndexFileString());

    QDomDocument doc("document");
    QFile file(qApp->temporaryIndexFileString());
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
            const QString& targetUrl = qApp->baseURLString() + map + "/" + factionName + "/" + fileName + ".png";
            QNetworkReply* reply = _manager.get(QNetworkRequest(QUrl(targetUrl)));
            _replyFileNameMap.insert(reply, target);
            connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        }
        mapsDir.cdUp();
    }
    QNetworkReply* reply = _manager.get(QNetworkRequest(QUrl(qApp->baseURLString() + map + "/" + map + ".xml")));
    _replyFileNameMap.insert(reply, mapsDir.absolutePath() + "/" + map + ".xml");
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));

    QSettings* remote = qApp->remoteSettings();
    remote->beginGroup("Maps");
    QFile versionFile(qApp->localMapVersionFileString(map));
    versionFile.open(QIODevice::WriteOnly);
    versionFile.write(remote->value(map).toString().toLocal8Bit());
    remote->endGroup();
}

void SettingsWidget::downloadFinished() {
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(QObject::sender());

    const QString& filename = _replyFileNameMap.take(reply);

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    _statusText->append("Downloaded " + filename);

    if (_replyFileNameMap.keys().isEmpty()) {
        createMapEntries();
        emit finishedDownloadingMap(_currentMapDownload);
         setButtonsEnabled(true);
    }
}

void SettingsWidget::createMapEntries() {
    foreach (const MapEntry* mapEntry, _maps)
        delete mapEntry;
    _maps.clear();

    QSettings* remoteSettings = qApp->remoteSettings();

    remoteSettings->beginGroup("Maps");
    QStringList allMaps = remoteSettings->allKeys();

    if (!_layout)
        _layout = new QGridLayout(this);

    if (!_nameLabel) {
        _nameLabel = new QLabel("Name");
        _layout->addWidget(_nameLabel, 0, 0);
    }

    if (!_localVersionLabel) {
        _localVersionLabel = new QLabel("Local Version");
        _layout->addWidget(_localVersionLabel, 0, 1);
    }

    if (!_remoteVersionLabel) {
        _remoteVersionLabel = new QLabel("Remote Version");
        _layout->addWidget(_remoteVersionLabel, 0, 2);
    }

    int i = 0;
    remoteSettings->endGroup();
    for (; i < allMaps.size(); ++i) {
        const QString& map = allMaps[i];
        MapEntry* mapEntry = new MapEntry(map);
        setupMapEntry(mapEntry, remoteSettings);
        _maps.append(mapEntry);
        _layout->addWidget(mapEntry->nameLabel, i+1, 0);
        _layout->addWidget(mapEntry->localVersionLabel, i+1, 1);
        _layout->addWidget(mapEntry->remoteVersionLabel, i+1, 2);
        _layout->addWidget(mapEntry->actionButton, i+1, 3);
    }


    if (!_statusText) {
        _statusText = new QTextEdit;
        _statusText->setReadOnly(true);
        _layout->addWidget(_statusText, i+1, 0, 3, 0);
        _layout->setRowStretch(i+1, 1);
    }
}

void SettingsWidget::setButtonsEnabled(bool enabled) {
    foreach (MapEntry* map, _maps) {
        map->actionButton->setEnabled(enabled);
    }
}
