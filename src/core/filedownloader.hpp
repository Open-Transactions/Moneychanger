#ifndef FILEDOWNLOADER_HPP
#define FILEDOWNLOADER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>


#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);

    virtual ~FileDownloader();

    QByteArray downloadedData() const;

signals:
        void downloaded();

private slots:

    void fileDownloaded(QNetworkReply* pReply);

private:

    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;

};

#endif // FILEDOWNLOADER_HPP
