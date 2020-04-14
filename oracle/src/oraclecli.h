#ifndef ORACLECLI_H
#define ORACLECLI_H

#include <QJsonObject>
#include <QString>
#include <QVector>
#include <functional>

#include "pagetemplates.h"

class QNetworkAccessManager;
class QSharedMemory;

class ResourceToCheck
{
public:
    ResourceToCheck(QString _type, QString _url, std::function<const QString (QByteArray)> _parseCallback)
        : type(_type), url(_url), parseCallback(_parseCallback) {}

    const QString &getType() { return type; }
    const QString &getUrl() { return url; }
    std::function<const QString (QByteArray)> parseCallback;

private:
    QString type;
    QString url;
};

class OracleCli : public QObject
{
    Q_OBJECT
public:
    explicit OracleCli(QObject *parent = nullptr);

public:
    QNetworkAccessManager *nam;
    QVector<ResourceToCheck *> requests;
    QJsonObject results;
    ResourceToCheck *currentRequest;
    QSharedMemory *shm;

protected:
    void processNextRequest();
    void returnResults();
    void downloadFile(QUrl url);
    const QString checkCardsUpdate(QByteArray data);
    const QString checkTokensUpdate(QByteArray data);
    const QString checkSpoilersUpdate(QByteArray data);

signals:
    void finished();

public slots:
    void run();

protected slots:
    void actDownloadFinished();
};

#endif