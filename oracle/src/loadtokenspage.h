#ifndef LOADTOKENSPAGE_H
#define LOADTOKENSPAGE_H

#include "pagetemplates.h"

class LoadTokensPage : public SimpleDownloadFilePage
{
    Q_OBJECT
public:
    explicit LoadTokensPage(QWidget * = nullptr){};
    void retranslateUi() override;

protected:
    QString getDefaultUrl() override;
    QString getCustomUrlSettingsKey() override;
    QString getDefaultSavePath() override;
    QString getWindowTitle() override;
    QString getFileType() override;
};

#endif