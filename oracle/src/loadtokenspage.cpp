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
#include <QtGui>

#include "settingscache.h"
#include "loadtokenspage.h"

#define TOKENS_DOWNLOAD_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Token/master/tokens.xml"

QString LoadTokensPage::getDefaultUrl()
{
    return TOKENS_DOWNLOAD_URL;
}

QString LoadTokensPage::getCustomUrlSettingsKey()
{
    return "tokensurl";
}

QString LoadTokensPage::getDefaultSavePath()
{
    return settingsCache->getTokenDatabasePath();
}

QString LoadTokensPage::getWindowTitle()
{
    return tr("Save token database");
}

QString LoadTokensPage::getFileType()
{
    return tr("XML; token database (*.xml)");
}

void LoadTokensPage::retranslateUi()
{
    setTitle(tr("Tokens import"));
    setSubTitle(tr("Please specify a compatible source for token data."));

    urlLabel->setText(tr("Download URL:"));
    urlButton->setText(tr("Restore default URL"));
    pathLabel->setText(tr("The token database will be saved at the following location:") + "<br>" +
                       getDefaultSavePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));
}
