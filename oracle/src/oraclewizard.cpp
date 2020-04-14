#include "oraclewizard.h"

#include "loadcardspage.h"
#include "loadspoilerspage.h"
#include "loadtokenspage.h"
#include "main.h"
#include "settingscache.h"
#include "version_string.h"

#include <QAbstractButton>
#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTextEdit>
#include <QtGui>

OracleWizard::OracleWizard(QWidget *parent) : QWizard(parent)
{
    settings = new QSettings(settingsCache->getSettingsPath() + "global.ini", QSettings::IniFormat, this);
    connect(settingsCache, SIGNAL(langChanged()), this, SLOT(updateLanguage()));

    nam = new QNetworkAccessManager(this);

    addPage(new IntroPage);

    if (updateCards) {
        addPage(new LoadCardsPage);
    }

    if (updateTokens) {
        addPage(new LoadTokensPage);
    }

    if (updateSpoilers) {
        addPage(new LoadSpoilersPage);
    }

    addPage(new OutroPage);

    retranslateUi();
}

void OracleWizard::updateLanguage()
{
    qApp->removeTranslator(translator);
    installNewTranslator();
}

void OracleWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }

    QDialog::changeEvent(event);
}

void OracleWizard::retranslateUi()
{
    setWindowTitle(tr("Oracle Importer"));
    for (int i = 0; i < pageIds().count(); i++) {
        dynamic_cast<OracleWizardPage *>(page(i))->retranslateUi();
    }
}

void OracleWizard::accept()
{
    QDialog::accept();
}

void OracleWizard::enableButtons()
{
    button(QWizard::NextButton)->setDisabled(false);
    button(QWizard::BackButton)->setDisabled(false);
}

void OracleWizard::disableButtons()
{
    button(QWizard::NextButton)->setDisabled(true);
    button(QWizard::BackButton)->setDisabled(true);
}

bool OracleWizard::saveTokensToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "File open (w) failed for" << fileName;
        return false;
    }

    if (file.write(tokensData) == -1) {
        qDebug() << "File write (w) failed for" << fileName;
        return false;
    }

    file.close();
    return true;
}

IntroPage::IntroPage(QWidget *parent) : OracleWizardPage(parent)
{
    label = new QLabel(this);
    label->setWordWrap(true);

    languageLabel = new QLabel(this);
    versionLabel = new QLabel(this);
    languageBox = new QComboBox(this);
    QString setLanguage = settingsCache->getLang();

    QStringList qmFiles = findQmFiles();
    for (int i = 0; i < qmFiles.size(); i++) {
        QString langName = languageName(qmFiles[i]);
        languageBox->addItem(langName, qmFiles[i]);
        if ((qmFiles[i] == setLanguage) ||
            (setLanguage.isEmpty() && langName == QCoreApplication::translate("i18n", DEFAULT_LANG_NAME))) {
            languageBox->setCurrentIndex(i);
        }
    }

    connect(languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageBoxChanged(int)));

    auto *layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(languageLabel, 1, 0);
    layout->addWidget(languageBox, 1, 1);
    layout->addWidget(versionLabel, 2, 0, 1, 2);

    setLayout(layout);
}

QStringList IntroPage::findQmFiles()
{
    QDir dir(translationPath);
    QStringList fileNames = dir.entryList(QStringList(translationPrefix + "_*.qm"), QDir::Files, QDir::Name);
    fileNames.replaceInStrings(QRegExp(translationPrefix + "_(.*)\\.qm"), "\\1");
    return fileNames;
}

QString IntroPage::languageName(const QString &qmFile)
{
    if (qmFile == DEFAULT_LANG_CODE) {
        return DEFAULT_LANG_NAME;
    }

    QTranslator translator;
    translator.load(translationPrefix + "_" + qmFile + ".qm", translationPath);

    return translator.translate("i18n", DEFAULT_LANG_NAME);
}

void IntroPage::languageBoxChanged(int index)
{
    settingsCache->setLang(languageBox->itemData(index).toString());
}

void IntroPage::retranslateUi()
{
    setTitle(tr("Introduction"));
    label->setText(tr("This wizard will import the list of sets, cards, and tokens "
                      "that will be used by Cockatrice."));
    languageLabel->setText(tr("Interface language:"));
    versionLabel->setText(tr("Version:") + QString(" %1").arg(VERSION_STRING));
}

void OutroPage::retranslateUi()
{
    setTitle(tr("Finished"));
    setSubTitle(tr("The wizard has finished.") + "<br>" +
                tr("You can now start using Cockatrice with the newly updated cards.") + "<br><br>" +
                tr("If the card databases don't reload automatically, restart the Cockatrice client."));
}
