#pragma once

#include <QObject>

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QUrl>
#include <QMap>

namespace Chess {

    class DownloadManager : public QObject
    {
        Q_OBJECT
            QNetworkAccessManager manager;
        QList<QNetworkReply*> currentDownloads;
        QMap<QUrl, QString> destinationPaths;

    public:
        explicit DownloadManager(QObject* parent = nullptr);
        void doDownload(const QUrl& url);
        void doDownloadToPath(const QUrl& url, const QString& path);
        QString saveFileName(const QUrl& url);
        bool saveToDisk(const QString& filename, QIODevice* data);

    public slots:
        void execute(QStringList args);
        void downloadFinished();

    signals:
        void onDownloadFinished(QUrl, QString);
        void downloadError(QUrl);
        void downloadManagerIdle();

    };

}