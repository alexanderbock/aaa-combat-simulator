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

#include "simulatorapplication.h"

#include "combatwidget.h"
#include "settingswidget.h"
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QString>
#include <QTabWidget>
#include <QTextStream>

SimulatorApplication::SimulatorApplication(int& argc, char** argv)
    : QApplication(argc, argv)
    , _networkManager(new QNetworkAccessManager)
    , _localSettings(new QSettings)
    , _remoteSettings(nullptr)
    , _versionDownloadErrorOccurred(false)
    , _mainWidget(new QTabWidget)
{
    // Set general application information
    setApplicationName("AAACombatSimulator");
    setApplicationVersion("1.0");
    setOrganizationDomain("alexbock.dyndns.org");
    setOrganizationName("Bock");

    // start the download of the remote version file
    QNetworkReply* reply = _networkManager->get(QNetworkRequest(getRemoteVersionFileUrl()));
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(remoteVersionDownloadFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(remoteVersionDownloadFailed(QNetworkReply::NetworkError)));

    // create the necessary directory structure (%USER%/triplea/combatsim/) if it doesn't already exist and set it as the current
    QDir dir = mapsDirectory();
    dir.mkpath(".");
    QDir::setCurrent(dir.absolutePath());

    // create the widgets for the application
    _mainWidget->setMinimumSize(800, 640);
    foreach (const QString& dir, getListOfLocalMaps()) {
        const QDir& mapsDir = mapsDirectory(dir);
        if (mapsDir.exists() && mapsDir.exists(dir + ".xml")) {
            addTab(dir);
        }
    }
    SettingsWidget* mapsWidget = new SettingsWidget;
    connect(mapsWidget, SIGNAL(finishedDownloadingMap(QString)), this, SLOT(addTab(QString)));
    connect(mapsWidget, SIGNAL(finishedRemovingMap(QString)), this, SLOT(removeTab(QString)));
    _mainWidget->addTab(mapsWidget, "Settings");

    // lastly, select the previously selected tab and show the main widget 
    restoreState();
    _mainWidget->show();
}

SimulatorApplication::~SimulatorApplication() {
    saveState();
    delete _networkManager;
    delete _localSettings;
    delete _remoteSettings;
    delete _mainWidget;
}

void SimulatorApplication::addTab(QString name) {
    CombatWidget* widget = new CombatWidget(name);
    // if no tab exists yet, just add it
    if (_mainWidget->count() == 0) {
        _mainWidget->addTab(widget, name);
        return;
    }
    // find the correct spot for inserting the tab
    for (int i = 0; i < _mainWidget->count(); ++i) {
        CombatWidget* current = dynamic_cast<CombatWidget*>(_mainWidget->widget(i));
        if (!current) {
            _mainWidget->insertTab(i, widget, name);
            return; // if the widget is no CombatWidget, we have found the Settings Widget
        }
        const QString& currentName = current->directory();
        if (currentName.compare(name, Qt::CaseInsensitive) > 0) {
            _mainWidget->insertTab(i, widget, name);
            return;
        }
    }

    // if we got until here, the tab must be added to the back and no Settings Widget exists yet
    _mainWidget->addTab(widget, widget->directory());
}

void SimulatorApplication::removeTab(QString name) {
    int index = -1;
    for (int i = 0; i < _mainWidget->count(); ++i) {
        const QString& tabText = _mainWidget->tabText(i);
        if (name.compare(tabText, Qt::CaseInsensitive) == 0) {
            index = i;
            break;
        }
    }
    if (index != -1)
        _mainWidget->removeTab(index);
}

void SimulatorApplication::restoreState() {
    // select the old tab
    QString oldTabName = _localSettings->value("oldTab").toString();
    for (int i = 0; i < _mainWidget->count(); ++i) {
        const QString& tabText = _mainWidget->tabText(i);
        if (oldTabName.compare(tabText, Qt::CaseInsensitive) == 0) {
            _mainWidget->setCurrentIndex(i);
            break;
        }
    }
}

void SimulatorApplication::saveState() {
    int currentIndex = _mainWidget->currentIndex();
    QString tabText = _mainWidget->tabText(currentIndex);
    _localSettings->setValue("oldTab", tabText);
}

void SimulatorApplication::remoteVersionDownloadFinished() {
    if (!_versionDownloadErrorOccurred) {
        QNetworkReply* currentReply = dynamic_cast<QNetworkReply*>(QObject::sender());
        QFile file(getTemporaryVersionFileString());
        file.open(QIODevice::WriteOnly);
        file.write(currentReply->readAll());
        file.close();

        currentReply->deleteLater();

        _remoteSettings = new QSettings(getTemporaryVersionFileString(), QSettings::IniFormat);
        checkApplicationAndMapVersions();
        emit remoteVersionFileArrived();
    }
}

void SimulatorApplication::remoteVersionDownloadFailed(QNetworkReply::NetworkError) {
    _versionDownloadErrorOccurred = true;
}

void SimulatorApplication::checkApplicationAndMapVersions() const {
    // check general app version
    int result = compareVersions(_remoteSettings->value("Version").toString(), applicationVersion());
    bool newAppVersion = result > 0;

    _remoteSettings->beginGroup("Maps");
    QList<MapVersionInformation> newMapList;
    QDir mapsDir = mapsDirectory();
    QStringList dirs = mapsDir.entryList();
    foreach (const QString& dirName, dirs) {
        QDir dir(mapsDirectory(dirName));
        if (dir.exists("VERSION")) {
            QString localVersion = localMapVersion(dirName);
            QString remoteVersion = _remoteSettings->value(dirName).toString();
            result = compareVersions(remoteVersion, localVersion);
            if (result > 0) {
                MapVersionInformation info;
                info.name = dirName;
                info.localVersion = localVersion;
                info.remoteVersion = remoteVersion;
                newMapList.append(info);
            }
        }
    }
    _remoteSettings->endGroup();

    if (newAppVersion || !newMapList.isEmpty()) {
        QString text;
        if (newAppVersion)
            text = "<h3>New application version</h3><p>Current version: " + applicationVersion() + "<br>New version: " +
            _remoteSettings->value("Version").toString() +"</p><p><a href=\"" + getDownloadUrlString() + "\">Download</a><br>\
                                            <a href=\"" + getChangelogUrlString() + "\">Changelog</a></p>";

        if (!newMapList.isEmpty()) {
            text += "<h3>New map versions available</h3>";

            foreach (const MapVersionInformation& info, newMapList)
                text += "<p><b>" + info.name + "</b></p>";
        }
        QMessageBox msgBox(QMessageBox::Information, "New version available", text);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }
}

QSettings* SimulatorApplication::localSettings() const {
    return _localSettings;
}

QSettings* SimulatorApplication::remoteSettings() const {
    return _remoteSettings;
}

QNetworkAccessManager* SimulatorApplication::networkAccessManager() const {
    return _networkManager;
}

void SimulatorApplication::separateVersion(QString versionString,
                                           QString& major, QString& minor, QString& release) const
{
    // set default values for early function bailout
    major = "0";
    minor = "0";
    release = "0";

    QStringList versions = versionString.split(".");

    if (versions.size() > 0) {
        major = versions[0];

        if (versions.size() > 1) {
            minor = versions[1];

            if (versions.size() > 2)
                release = versions[2];
        }
    }
}

int SimulatorApplication::compareVersions(const QString& v1, const QString& v2) const {
    // assume there is a version format like 
    // [0-9].[0-9].[0-9]     i.e.  Major.Minor.Release

    QString v1Major, v1Minor, v1Release;
    QString v2Major, v2Minor, v2Release;

    separateVersion(v1, v1Major, v1Minor, v1Release);
    separateVersion(v2, v2Major, v2Minor, v2Release);

    int major = v1Major.compare(v2Major);
    int minor = v1Minor.compare(v2Minor);
    int release = v1Release.compare(v2Release);

    int total = 100 * major + 10 * minor + release;
    if (total < 0)
        return -1;
    else if (total > 0)
        return 1;
    else
        return 0;
}

QString SimulatorApplication::localMapVersion(const QString& map) const {
    QString file = mapsDirectory(map).absolutePath() + "/VERSION";
    QFile versionFile(file);
    versionFile.open(QIODevice::ReadOnly);
    QTextStream stream(&versionFile);
    QString localVersion = stream.readAll();
    return localVersion;
}

QStringList SimulatorApplication::getListOfLocalMaps() const {
    QDir current = mapsDirectory();
    current.setFilter(QDir::Dirs);
    return current.entryList();
}

QDir SimulatorApplication::mapsDirectory(const QString& subDir) const {
    return QDir(QDir::homePath() + "/triplea/combatsim/" + subDir + "/");
}

QUrl SimulatorApplication::getRemoteVersionFileUrl() const {
    return QUrl("http://webstaff.itn.liu.se/~alebo68/TripleA/VERSION");
}

QString SimulatorApplication::getTemporaryVersionFileString() const {
    return QDir::tempPath() + "/VERSION";
}

QString SimulatorApplication::getDownloadUrlString() const {
    return "http://webstaff.itn.liu.se/~alebo68/TripleA/AAACombatSimulator.exe";
}

QString SimulatorApplication::getChangelogUrlString() const {
    return "http://webstaff.itn.liu.se/~alebo68/TripleA/CHANGELOG";
}

QUrl SimulatorApplication::remoteMapIndexURL(const QString& map) const {
    return QUrl(baseURLString() + map + "/INDEX");
}

QString SimulatorApplication::temporaryIndexFileString() const {
    return QDir::tempPath() + "/INDEX";
}

QString SimulatorApplication::baseURLString() const {
    return "http://webstaff.itn.liu.se/~alebo68/TripleA/";
}

QString SimulatorApplication::localMapVersionFileString(const QString& map) const {
    return mapsDirectory(map).absolutePath() + "/VERSION";
}
