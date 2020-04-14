#include "main.h"

#include "oraclecli.h"
#include "oraclewizard.h"
#include "settingscache.h"
#include "thememanager.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTimer>
#include <QTranslator>

QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
ThemeManager *themeManager;

const QString translationPrefix = "oracle";
QString translationPath;
bool updateCards, updateTokens, updateSpoilers;

void installNewTranslator()
{
    QString lang = settingsCache->getLang();

    qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(qtTranslator);
    translator->load(translationPrefix + "_" + lang, translationPath);
    qApp->installTranslator(translator);
}

int main(int argc, char *argv[])
{
    bool checkUpdatesOnly;

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Cockatrice");
    QCoreApplication::setOrganizationDomain("cockatrice");
    // this can't be changed, as it influences the default save path for cards.xml
    QCoreApplication::setApplicationName("Cockatrice");

    QCommandLineParser parser;
    QCommandLineOption updateCardsOption(QStringList() << "c"
                                                       << "cards",
                                         QCoreApplication::translate("main", "Check/update cards"));
    QCommandLineOption updateTokensOption(QStringList() << "t"
                                                        << "tokens",
                                          QCoreApplication::translate("main", "Check/update tokens"));
    QCommandLineOption updateSpoilersOption(QStringList() << "s"
                                                          << "spoilers",
                                            QCoreApplication::translate("main", "Check/update spoilers"));
    QCommandLineOption checkUpdatesOnlyOption(QStringList() << "check-updates",
                                              QCoreApplication::translate("main", "Check updates CLI mode"));

    parser.addOption(updateCardsOption);
    parser.addOption(updateTokensOption);
    parser.addOption(updateSpoilersOption);
    parser.addOption(checkUpdatesOnlyOption);

    parser.process(app);
    updateCards = parser.isSet(updateCardsOption);
    updateTokens = parser.isSet(updateTokensOption);
    updateSpoilers = parser.isSet(updateSpoilersOption);
    checkUpdatesOnly = parser.isSet(checkUpdatesOnlyOption);

    if (!updateCards && !updateTokens && !updateSpoilers) {
        // no specific update requestes, run them all
        updateCards = true;
        updateTokens = true;
        updateSpoilers = true;
    }

#ifdef Q_OS_MAC
    translationPath = qApp->applicationDirPath() + "/../Resources/translations";
#elif defined(Q_OS_WIN)
    translationPath = qApp->applicationDirPath() + "/translations";
#else // linux
    translationPath = qApp->applicationDirPath() + "/../share/oracle/translations";
#endif

    settingsCache = new SettingsCache;
    themeManager = new ThemeManager;

    qtTranslator = new QTranslator;
    translator = new QTranslator;
    installNewTranslator();

    OracleCli cli;
    OracleWizard wizard;

    if (checkUpdatesOnly) {
        QObject::connect(&cli, SIGNAL(finished()), &app, SLOT(quit()));
        // run the app from the appication event loop
        QTimer::singleShot(0, &cli, SLOT(run()));
    } else {
        QIcon icon("theme:appicon.svg");
        wizard.setWindowIcon(icon);

        wizard.show();
    }

    return app.exec();
}
