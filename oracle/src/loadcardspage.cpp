#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QtConcurrent>
#include <QtGui>

#include "settingscache.h"
#include "oracleimporter.h"
#include "oraclewizard.h"
#include "loadcardspage.h"

#ifdef HAS_LZMA
#define MTGJSON_DOWNLOAD_URL "https://www.mtgjson.com/files/AllPrintings.json.xz"
#elif defined(HAS_ZLIB)
#define MTGJSON_DOWNLOAD_URL "https://www.mtgjson.com/files/AllPrintings.json.zip"
#else
#define MTGJSON_DOWNLOAD_URL "https://www.mtgjson.com/files/AllPrintings.json"
#endif

#define MTGJSON_VERSION_URL "https://www.mtgjson.com/files/version.json"

LoadCardsPage::LoadCardsPage(QWidget *)
{
    importer = new OracleImporter(settingsCache->getDataPath(), this);
    connect(importer, SIGNAL(internalParseProgress(int, int)), this, SLOT(internalParseProgress(int, int)));
}

LoadCardsPage::~LoadCardsPage()
{
    disconnect(importer, SIGNAL(internalParseProgress(int, int)));
    importer->deleteLater();
}

QString LoadCardsPage::getDefaultUrl()
{
    return MTGJSON_DOWNLOAD_URL;
}

QString LoadCardsPage::getCustomUrlSettingsKey()
{
    return "allsetsurl";
}

QString LoadCardsPage::getDefaultSavePath()
{
    return settingsCache->getCardDatabasePath();
}

QString LoadCardsPage::getWindowTitle()
{
    return tr("Save card database");
}

QString LoadCardsPage::getFileType()
{
    return tr("XML; card database (*.xml)");
}

void LoadCardsPage::retranslateUi()
{
    setTitle(tr("Cards import"));
    setSubTitle(tr("Please specify a compatible source for cards data."));

    urlLabel->setText(tr("Download URL:"));
    urlButton->setText(tr("Restore default URL"));
    pathLabel->setText(tr("The card database will be saved at the following location:") + "<br>" +
                       getDefaultSavePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));
}

void LoadCardsPage::internalParseData()
{
    if (urlLineEdit->text() == getDefaultUrl()) {
        QUrl versionUrl = QUrl::fromUserInput(MTGJSON_VERSION_URL);
        QNetworkReply *versionReply = wizard()->nam->get(QNetworkRequest(versionUrl));
        connect(versionReply, &QNetworkReply::finished, [this, versionReply]() {
            QString sourceUrl = urlLineEdit->text();
            QString sourceVersion = "unknown";

            if (versionReply->error() == QNetworkReply::NoError) {
                QByteArray jsonData = versionReply->readAll();
                QJsonParseError jsonError;
                QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonData, &jsonError);

                if (jsonError.error == QJsonParseError::NoError) {
                    QVariantMap jsonMap = jsonResponse.toVariant().toMap();
                    sourceVersion = jsonMap["version"].toString();
                    if (sourceVersion.isEmpty()) {
                        sourceVersion = "unknown";
                    }
                }
            }

            versionReply->deleteLater();
            internalStartComputation(sourceUrl, sourceVersion);
        });
    } else {
        QString sourceUrl = urlLineEdit->text();
        QString sourceVersion = "unknown";
        internalStartComputation(sourceUrl, sourceVersion);
    }
}

void LoadCardsPage::internalStartComputation(const QString sourceUrl, const QString sourceVersion)
{
    // Force downloadData to be passed by reference
    future = QtConcurrent::run(importer, &OracleImporter::startImport, std::ref(downloadData), sourceUrl, sourceVersion);
    connect(&watcher, SIGNAL(finished()), this, SLOT(internalParseFinished()));
    watcher.setFuture(future);
}
