#include "carddatabase.h"
#include "cardupdater.h"
#include "main.h"
#include "settingscache.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QSharedMemory>
#include <QtConcurrent>

#define SHM_KEY "cockatrice_oracle_shm"

QProcess *CardUpdater::cardUpdateProcess = nullptr;

CardUpdater::CardUpdater(QWidget *parent)
: QWidget(parent), shm(nullptr)
{
}

const QString CardUpdater::getUpdaterBinaryPath()
{
    // full "run the update" command; leave empty if not present
    QString updaterCmd;
    QString binaryName;
    QDir dir = QDir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    /*
     * bypass app translocation: quarantined application will be started from a temporary directory eg.
     * /private/var/folders/tk/qx76cyb50jn5dvj7rrgfscz40000gn/T/AppTranslocation/A0CBBD5A-9264-4106-8547-36B84DB161E2/d/oracle/
     */
    if (dir.absolutePath().startsWith("/private/var/folders")) {
        dir.setPath("/Applications/");
    } else {
        // exit from the Cockatrice application bundle
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
    }

    binaryName = getCardUpdaterBinaryName();

    dir.cd(binaryName + ".app");
    dir.cd("Contents");
    dir.cd("MacOS");
#elif defined(Q_OS_WIN)
    binaryName = getCardUpdaterBinaryName() + ".exe";
#else
    binaryName = getCardUpdaterBinaryName();
#endif

    if (dir.exists(binaryName)) {
        updaterCmd = dir.absoluteFilePath(binaryName);
    }

    if (updaterCmd.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Unable to run the card database updater: ") + dir.absoluteFilePath(binaryName));
    }

    return updaterCmd;
}

const QStringList CardUpdater::getUpdaterParameters(bool checkOnly)
{
    QStringList parameters;
    if(settingsCache->getCheckCardUpdates()) {
        parameters << "-c";
    }
    if(settingsCache->getCheckTokenUpdates()) {
        parameters << "-t";
    }
    if(settingsCache->getCheckSpoilerUpdates()) {
        parameters << "-s";
    }

    if(checkOnly) {
        parameters << "--check-updates";
    }
    return parameters;
}

void CardUpdater::actCheckCardUpdates(bool checkOnly)
{
    if (cardUpdateProcess) {
        QMessageBox::information(this, tr("Information"), tr("A card database update is already running."));
        deleteLater();
        return;
    }

    QString updaterCmd = getUpdaterBinaryPath();
    if (updaterCmd.isEmpty()) {
        deleteLater();
        return;
    }
    QStringList parameters = getUpdaterParameters(checkOnly);

    cardUpdateProcess = new QProcess(this);
    connect(cardUpdateProcess, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(cardUpdateError(QProcess::ProcessError)));
    if(checkOnly) {
        shm = new QSharedMemory(SHM_KEY, this);
        if(!shm->create(1024, QSharedMemory::ReadOnly))
        {
            qDebug() << "Unable to create shared memory segment" << shm->error();
        }

        connect(cardUpdateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(cardCheckFinished(int, QProcess::ExitStatus)));
    } else {
        connect(cardUpdateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(cardUpdateFinished(int, QProcess::ExitStatus)));
    }

    qDebug() << "Running updater:" << updaterCmd << parameters;
    cardUpdateProcess->setProgram(updaterCmd);
    cardUpdateProcess->setArguments(parameters);
    cardUpdateProcess->start();
}

void CardUpdater::cardUpdateError(QProcess::ProcessError err)
{
    QString error, error2;
    switch (err) {
        case QProcess::FailedToStart:
            error = tr("failed to start.");
            break;
        case QProcess::Crashed:
            error = tr("crashed.");
            break;
        case QProcess::Timedout:
            error = tr("timed out.");
            break;
        case QProcess::WriteError:
            error = tr("write error.");
            break;
        case QProcess::ReadError:
            error = tr("read error.");
            break;
        case QProcess::UnknownError:
        default:
            error = tr("unknown error.");
            break;
    }

    error2 = cardUpdateProcess->errorString();

    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    deleteLater();

    QMessageBox::warning(this, tr("Error"), tr("The card database updater exited with an error: %1 %2").arg(error).arg(error2));
}

void CardUpdater::cardUpdateFinished(int, QProcess::ExitStatus)
{
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    deleteLater();

    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
}


void CardUpdater::cardCheckFinished(int, QProcess::ExitStatus)
{
    QByteArray result((const char *)shm->data(), shm->size());
    auto index = result.indexOf('\0');
    if (index != -1) {
        result.truncate(index);
    }

    QJsonParseError parseError;

    QString newCardVersion, newTokenVersion, newSpoilerVersion;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(result, &parseError);
    if (parseError.error == QJsonParseError::NoError) {
        QVariantMap jsonMap = jsonResponse.toVariant().toMap();
        newCardVersion = jsonMap["c"].toString();
        newTokenVersion = jsonMap["t"].toString();
        newSpoilerVersion = jsonMap["s"].toString();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Unable to interpret data returned by the updater"));
    }

    QString oldCardVersion, oldTokenVersion, oldSpoilerVersion;
    QVector<CardSourceInfo> currentCardSource = db->getCardSources();
    for(CardSourceInfo info : currentCardSource) {
        if(info.cardSourceType == "c")
        {
            oldCardVersion = info.sourceVersion;
        } else if(info.cardSourceType == "t") {
            oldTokenVersion = info.sourceVersion;
        } else if(info.cardSourceType == "s") {
            oldSpoilerVersion = info.sourceVersion;
        }
    }

    qDebug() << "SOURCE" << "OLD" << "NEW";
    qDebug() << "card" << oldCardVersion << newCardVersion;
    qDebug() << "token" << oldTokenVersion << newTokenVersion;
    qDebug() << "spoiler" << oldSpoilerVersion << newSpoilerVersion;

    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    deleteLater();
}
