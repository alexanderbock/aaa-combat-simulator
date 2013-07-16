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
    , networkManager_(0)
    , localSettings_(0)
    , remoteSettings_(0)
    , versionDownloadErrorOccurred_(false)
    , mainWidget_(0)
{
    // Set general application information
    setApplicationName("AAACombatSimulator");
    setApplicationVersion("1.0");
    setOrganizationDomain("webstaff.itn.liu.se/~alebo68");
    setOrganizationName("Bock");

    // initializing member items
    networkManager_ = new QNetworkAccessManager;
    localSettings_ = new QSettings;

    // start the download of the remote version file
    QNetworkReply* reply = networkManager_->get(QNetworkRequest(getRemoteVersionFileUrl()));
    QObject::connect(reply, SIGNAL(finished()), this, SLOT(remoteVersionDownloadFinished()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(remoteVersionDownloadFailed(QNetworkReply::NetworkError)));

    // create the necessary directory structure (%USER%/triplea/combatsim/) if it doesn't already exist and set it as the current
    QDir dir = getMapsDirectory();
    dir.mkpath(".");
    QDir::setCurrent(dir.absolutePath());

    // create the widgets for the application
    mainWidget_ = new QTabWidget;
    mainWidget_->setMinimumSize(800, 640);
    foreach (const QString& dir, getListOfLocalMaps()) {
        const QDir& mapsDir = getMapsDirectory(dir);
        if (mapsDir.exists() && mapsDir.exists(dir + ".xml")) {
            addTab(dir);
        }
    }
    SettingsWidget* mapsWidget = new SettingsWidget;
    connect(mapsWidget, SIGNAL(finishedDownloadingMap(QString)), this, SLOT(addTab(QString)));
    connect(mapsWidget, SIGNAL(finishedRemovingMap(QString)), this, SLOT(removeTab(QString)));
    mainWidget_->addTab(mapsWidget, "Settings");

    // lastly, select the previously selected tab and show the main widget 
    restoreState();
    mainWidget_->show();
}

SimulatorApplication::~SimulatorApplication() {
    saveState();
    delete networkManager_;
    delete localSettings_;
    delete remoteSettings_;
    delete mainWidget_;
}

void SimulatorApplication::addTab(QString name) {
    CombatWidget* widget = new CombatWidget(name);
    // if no tab exists yet, just add it
    if (mainWidget_->count() == 0) {
        mainWidget_->addTab(widget, name);
        return;
    }
    // find the correct spot for inserting the tab
    for (int i = 0; i < mainWidget_->count(); ++i) {
        CombatWidget* current = dynamic_cast<CombatWidget*>(mainWidget_->widget(i));
        if (!current) {
            mainWidget_->insertTab(i, widget, name);
            return; // if the widget is no CombatWidget, we have found the Settings Widget
        }
        const QString& currentName = current->getDirectory();
        if (currentName.compare(name, Qt::CaseInsensitive) > 0) {
            mainWidget_->insertTab(i, widget, name);
            return;
        }
    }

    // if we got until here, the tab must be added to the back and not Settings Widget exists yet
    mainWidget_->addTab(widget, widget->getDirectory());
}

void SimulatorApplication::removeTab(QString name) {
    int index = -1;
    for (int i = 0; i < mainWidget_->count(); ++i) {
        const QString& tabText = mainWidget_->tabText(i);
        if (name.compare(tabText, Qt::CaseInsensitive) == 0) {
            index = i;
            break;
        }
    }
    if (index != -1)
        mainWidget_->removeTab(index);
}

void SimulatorApplication::restoreState() {
    // select the old tab
    QString oldTabName = localSettings_->value("oldTab").toString();
    for (int i = 0; i < mainWidget_->count(); ++i) {
        const QString& tabText = mainWidget_->tabText(i);
        if (oldTabName.compare(tabText, Qt::CaseInsensitive) == 0) {
            mainWidget_->setCurrentIndex(i);
            break;
        }
    }
}

void SimulatorApplication::saveState() {
    int currentIndex = mainWidget_->currentIndex();
    QString tabText = mainWidget_->tabText(currentIndex);
    localSettings_->setValue("oldTab", tabText);
}

void SimulatorApplication::remoteVersionDownloadFinished() {
    if (!versionDownloadErrorOccurred_) {
        QNetworkReply* currentReply = dynamic_cast<QNetworkReply*>(QObject::sender());
        QFile file(getTemporaryVersionFileString());
        file.open(QIODevice::WriteOnly);
        file.write(currentReply->readAll());
        file.close();

        currentReply->deleteLater();

        remoteSettings_ = new QSettings(getTemporaryVersionFileString(), QSettings::IniFormat);
        checkApplicationAndMapVersions();
        emit remoteVersionFileArrived();
    }
}

void SimulatorApplication::remoteVersionDownloadFailed(QNetworkReply::NetworkError) {
    versionDownloadErrorOccurred_ = true;
}

void SimulatorApplication::checkApplicationAndMapVersions() const {
    // check general app version
    int result = compareVersions(remoteSettings_->value("Version").toString(), applicationVersion());
    bool newAppVersion = result > 0;

    remoteSettings_->beginGroup("Maps");
    QList<MapVersionInformation> newMapList;
    QDir mapsDir = getMapsDirectory();
    QStringList dirs = mapsDir.entryList();
    foreach (const QString& dirName, dirs) {
        QDir dir(getMapsDirectory(dirName));
        if (dir.exists("VERSION")) {
            QString localVersion = getLocalMapVersion(dirName);
            QString remoteVersion = remoteSettings_->value(dirName).toString();
            result = compareVersions(remoteVersion, localVersion);
            if (result > 0) {
                MapVersionInformation info;
                info.name_ = dirName;
                info.localVersion_ = localVersion;
                info.remoteVersion_ = remoteVersion;
                newMapList.append(info);
            }
        }
    }
    remoteSettings_->endGroup();

    if (newAppVersion || !newMapList.isEmpty()) {
        QString text;
        if (newAppVersion)
            text = "<h3>New application version</h3><p>Current version: " + applicationVersion() + "<br>New version: " +
            remoteSettings_->value("Version").toString() +"</p><p><a href=\"" + getDownloadUrlString() + "\">Download</a><br>\
                                            <a href=\"" + getChangelogUrlString() + "\">Changelog</a></p>";

        if (!newMapList.isEmpty()) {
            text += "<h3>New map versions available</h3>";

            foreach (const MapVersionInformation& info, newMapList)
                text += "<p><b>" + info.name_ + "</b></p>";
        }
        QMessageBox msgBox(QMessageBox::Information, "New version available", text);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }
}

QSettings* SimulatorApplication::getLocalSettings() const {
    return localSettings_;
}

QSettings* SimulatorApplication::getRemoteSettings() const {
    return remoteSettings_;
}

QNetworkAccessManager* SimulatorApplication::getNetworkAccessManager() const {
    return networkManager_;
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

QString SimulatorApplication::getLocalMapVersion(const QString& map) const {
    QString file = getMapsDirectory(map).absolutePath() + "/VERSION";
    QFile versionFile(file);
    versionFile.open(QIODevice::ReadOnly);
    QTextStream stream(&versionFile);
    QString localVersion = stream.readAll();
    return localVersion;
}

QStringList SimulatorApplication::getListOfLocalMaps() const {
    QDir current = getMapsDirectory();
    current.setFilter(QDir::Dirs);
    return current.entryList();
}

QDir SimulatorApplication::getMapsDirectory(const QString& subDir) const {
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

QUrl SimulatorApplication::getRemoteMapIndexUrl(const QString& map) const {
    return QUrl(getBaseUrlString() + map + "/INDEX");
}

QString SimulatorApplication::getTemporaryIndexFileString() const {
    return QDir::tempPath() + "/INDEX";
}

QString SimulatorApplication::getBaseUrlString() const {
    return "http://webstaff.itn.liu.se/~alebo68/TripleA/";
}

QString SimulatorApplication::getLocalMapVersionFileString(const QString& map) const {
    return getMapsDirectory(map).absolutePath() + "/VERSION";
}
