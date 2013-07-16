#ifndef BOCK_SETTINGSWIDGET_H
#define BOCK_SETTINGSWIDGET_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QSettings>
#include <QString>
#include <QUrl>

class QCheckBox;
class QGridLayout;
class QLabel;
class QPushButton;
class QTextEdit;

void downloadFile(QNetworkAccessManager& manager, const QUrl& url, const QString& filename);

struct MapEntry {
    QString mapName_;
    QLabel* nameLabel_;
    QLabel* localVersionLabel_;
    QLabel* remoteVersionLabel_;
    QPushButton* actionButton_;

    MapEntry(const QString& mapName);
    ~MapEntry();
};

class SettingsWidget : public QWidget {
Q_OBJECT
public:
    SettingsWidget();

signals:
    void finishedDownloadingMap(QString);
    void finishedRemovingMap(QString);

public slots:
    void createMapEntries();

private slots:
    void buttonClicked();
    void downloadFinished();

private:
    bool isMapInstalled(const QString& map);
    bool needsUpdate(const QString& map);
    void removeMap(const QString& map);
    void downloadMap(const QString& map);
    void setupMapEntry(const MapEntry* mapEntry, QSettings* remote);
    void setButtonsEnabled(bool enabled);

    QNetworkAccessManager manager_;

    QGridLayout* layout_;
    QList<MapEntry*> maps_;
    QMap<QNetworkReply*, QString> replyFileNameMap_;
    QLabel* nameLabel_;
    QLabel* localVersionLabel_;
    QLabel* remoteVersionLabel_;
    QTextEdit* statusText_;
    QString currentMapDownload_;
};

#endif
