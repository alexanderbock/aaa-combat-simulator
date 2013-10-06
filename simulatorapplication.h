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

#ifndef BOCK_SIMULATORAPPLICATION_H
#define BOCK_SIMULATORAPPLICATION_H

#include <QApplication>
#include <QDir>
#include <QNetworkReply>
#include <QUrl>

class CombatWidget;
class SettingsWidget;
class QNetworkAccessManager;
class QSettings;
class QTabWidget;

#undef qApp
#define qApp (static_cast<SimulatorApplication*>(QCoreApplication::instance()))


class SimulatorApplication : public QApplication {
Q_OBJECT
public:
    SimulatorApplication(int& argc, char** argv);
    ~SimulatorApplication();

    QNetworkAccessManager* getNetworkAccessManager() const;
    QSettings* getLocalSettings() const;
    QSettings* getRemoteSettings() const;
    QDir getMapsDirectory(const QString& subDir = ".") const;
    QString getLocalMapVersion(const QString& map) const;
    QString getLocalMapVersionFileString(const QString& map) const;
    QUrl getRemoteMapIndexUrl(const QString& map) const;
    QString getTemporaryIndexFileString() const;
    QString getBaseUrlString() const;

public slots:
    void addTab(QString);
    void removeTab(QString);

private slots:
    void remoteVersionDownloadFinished();
    void remoteVersionDownloadFailed(QNetworkReply::NetworkError);

signals:
    void remoteVersionFileArrived();

private:
    struct MapVersionInformation {
        QString name;
        QString localVersion;
        QString remoteVersion;
    };
    
    void checkApplicationAndMapVersions() const;
    void separateVersion(QString versionString, QString& major, QString& minor, QString& release) const;
    int compareVersions(const QString& v1, const QString& v2) const; //< returns -1 if v1<v2   0 if v1==v2  and 1 if v1>v2
    QUrl getRemoteVersionFileUrl() const;
    QString getTemporaryVersionFileString() const;
    QString getDownloadUrlString() const;
    QString getChangelogUrlString() const;
    QStringList getListOfLocalMaps() const;
    void restoreState();
    void saveState();

    QTabWidget* _mainWidget;
    QNetworkAccessManager* _networkManager; //< central instance to apply for downloads
    QSettings* _localSettings;
    QSettings* _remoteSettings;

    bool _versionDownloadErrorOccurred; //< will be set to true if an error occurs during the download of the remote version file
};

#endif
