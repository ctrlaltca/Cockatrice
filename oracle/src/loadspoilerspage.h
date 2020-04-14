#ifndef LOADSPOILERSPAGE_H
#define LOADSPOILERSPAGE_H

#include "pagetemplates.h"

class LoadSpoilersPage : public SimpleDownloadFilePage
{
    Q_OBJECT
public:
    explicit LoadSpoilersPage(QWidget * = nullptr){};
    void retranslateUi() override;

protected:
    QString getDefaultUrl() override;
    QString getCustomUrlSettingsKey() override;
    QString getDefaultSavePath() override;
    QString getWindowTitle() override;
    QString getFileType() override;
};

#endif