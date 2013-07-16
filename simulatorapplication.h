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
        QString name_;
        QString localVersion_;
        QString remoteVersion_;
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

    QNetworkAccessManager* networkManager_; //< central instance to apply for downloads
    QSettings* localSettings_;
    QSettings* remoteSettings_;

    bool versionDownloadErrorOccurred_; //< will be set to true if an error occurs during the download of the remote version file

    QTabWidget* mainWidget_;
};

#endif
