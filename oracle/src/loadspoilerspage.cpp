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
#include "loadspoilerspage.h"

#define SPOILERS_DOWNLOAD_URL "https://raw.githubusercontent.com/Cockatrice/Magic-Spoiler/files/spoiler.xml"

QString LoadSpoilersPage::getDefaultUrl()
{
    return SPOILERS_DOWNLOAD_URL;
}

QString LoadSpoilersPage::getCustomUrlSettingsKey()
{
    return "spoilersurl";
}

QString LoadSpoilersPage::getDefaultSavePath()
{
    return settingsCache->getSpoilerDatabasePath();
}

QString LoadSpoilersPage::getWindowTitle()
{
    return tr("Save spoiler database");
}

QString LoadSpoilersPage::getFileType()
{
    return tr("XML; spoiler database (*.xml)");
}

void LoadSpoilersPage::retranslateUi()
{
    setTitle(tr("Spoilers import"));
    setSubTitle(tr("Please specify a compatible source for spoiler data."));

    urlLabel->setText(tr("Download URL:"));
    urlButton->setText(tr("Restore default URL"));
    pathLabel->setText(tr("The spoiler database will be saved at the following location:") + "<br>" +
                       getDefaultSavePath());
    defaultPathCheckBox->setText(tr("Save to a custom path (not recommended)"));
}
