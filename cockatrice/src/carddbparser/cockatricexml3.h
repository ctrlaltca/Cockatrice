#ifndef COCKATRICE_XML3_H
#define COCKATRICE_XML3_H

#include "carddatabaseparser.h"

#include <QXmlStreamReader>

class CockatriceXml3Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
public:
    CockatriceXml3Parser() = default;
    ~CockatriceXml3Parser() override = default;
    bool getCanParseFile(const QString &name, QIODevice &device) override;
    void parseFile(QIODevice &device, const QString &cardSourceType) override;
    bool internalSaveToIODevice(SetNameMap sets,
                                CardNameMap cards,
                                QIODevice &device,
                                const QString &sourceUrl,
                                const QString &sourceVersion) override;

private:
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
    void loadInfoFromXml(QXmlStreamReader &xml, const QString &cardSourceType);
    QString getMainCardType(QString &type);
signals:
    void addCard(CardInfoPtr card) override;
    void addSet(CardSetPtr set) override;
    void addInfo(CardSourceInfo info) override;
};

#endif