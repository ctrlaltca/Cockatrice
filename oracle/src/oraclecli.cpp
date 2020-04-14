#include "main.h"
#include "oraclecli.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSharedMemory>
#include <QTextStream>

#define SHM_KEY "cockatrice_oracle_shm"

#define MTGJSON_VERSION_URL "https://www.mtgjson.com/files/version.json"
#define TOKENS_VERSION_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/version.txt"
#define SPOILERS_VERSION_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/SpoilerSeasonEnabled"

OracleCli::OracleCli(QObject *parent)
: QObject(parent), currentRequest(nullptr), shm(nullptr)
{
    nam = new QNetworkAccessManager(this);
}

void OracleCli::run()
{
    if (updateCards) {
        requests << new ResourceToCheck(
            "c",
            MTGJSON_VERSION_URL,
            std::bind( &OracleCli::checkCardsUpdate, this, std::placeholders::_1)
        );
    }

    if (updateTokens) {
        requests << new ResourceToCheck(
            "t",
            TOKENS_VERSION_URL,
            std::bind( &OracleCli::checkTokensUpdate, this, std::placeholders::_1)
        );
    }

    if (updateSpoilers) {
        requests << new ResourceToCheck(
            "s",
            SPOILERS_VERSION_URL,
            std::bind( &OracleCli::checkSpoilersUpdate, this, std::placeholders::_1)
        );
    }

    processNextRequest();
}

void OracleCli::returnResults()
{
    QJsonDocument doc(results);
    QByteArray json = doc.toJson();
    QTextStream(stdout) << json;

    shm = new QSharedMemory(SHM_KEY, this);
    if(!shm->attach(QSharedMemory::ReadWrite))
    {
        qDebug() << "Unable to attach to shared memory segment" << shm->error();
    } else {
        shm->lock();
        char *to = (char*)shm->data();
        const char *from = json.data();
        memcpy(to, from, qMin(json.size(), shm->size()));
        shm->unlock();
    }

    emit finished(); 
}

void OracleCli::processNextRequest()
{
    if(requests.isEmpty())
    {
        returnResults();
        return;
    }

    currentRequest = requests.takeFirst();
    QUrl url = QUrl::fromUserInput(currentRequest->getUrl());
    downloadFile(url);
}

void OracleCli::downloadFile(QUrl url)
{
    QNetworkReply *reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinished()));
}

void OracleCli::actDownloadFinished()
{
    // check for a reply
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        results.insert(currentRequest->getType(), QString());
        processNextRequest();
        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        downloadFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    QString result = currentRequest->parseCallback(reply->readAll());
    results.insert(currentRequest->getType(), result);

    reply->deleteLater();
    processNextRequest();
}

const QString OracleCli::checkCardsUpdate(QByteArray data)
{
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        return QString();
    }

    QVariantMap jsonMap = jsonResponse.toVariant().toMap();
    QString versionString = jsonMap["version"].toString();
    return versionString;
}

const QString OracleCli::checkTokensUpdate(QByteArray data)
{
    return QString(data);
}

const QString OracleCli::checkSpoilersUpdate(QByteArray data)
{
    return QString(data);
}
