#ifndef LOADCARDSPAGE_H
#define LOADCARDSPAGE_H

#include "pagetemplates.h"

class OracleImporter;

class LoadCardsPage : public SimpleDownloadFilePage
{
    Q_OBJECT
public:
    explicit LoadCardsPage(QWidget * = nullptr);
    ~LoadCardsPage();
    void retranslateUi() override;

private:
	OracleImporter *importer;

protected:
    QString getDefaultUrl() override;
    QString getCustomUrlSettingsKey() override;
    QString getDefaultSavePath() override;
    QString getWindowTitle() override;
    QString getFileType() override;
	void internalParseData() override;
    void internalStartComputation(const QString sourceUrl, const QString sourceVersion);
};

#endif