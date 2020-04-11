#ifndef COCKATRICE_XML4_H
#define COCKATRICE_XML4_H

#include "carddatabaseparser.h"

#include <QXmlStreamReader>

class CockatriceXml4Parser : public ICardDatabaseParser
{
    Q_OBJECT
    Q_INTERFACES(ICardDatabaseParser)
public:
    CockatriceXml4Parser() = default;
    ~CockatriceXml4Parser() override = default;
    bool getCanParseFile(const QString &name, QIODevice &device) override;
    void parseFile(QIODevice &device, const QString &cardSourceType) override;
    bool internalSaveToIODevice(SetNameMap sets,
                                CardNameMap cards,
                                QIODevice &device,
                                const QString &sourceUrl,
                                const QString &sourceVersion) override;

private:
    QVariantHash loadCardPropertiesFromXml(QXmlStreamReader &xml);
    void loadCardsFromXml(QXmlStreamReader &xml);
    void loadSetsFromXml(QXmlStreamReader &xml);
    void loadInfoFromXml(QXmlStreamReader &xml, const QString &cardSourceType);
signals:
    void addCard(CardInfoPtr card) override;
    void addSet(CardSetPtr set) override;
    void addInfo(CardSourceInfo info) override;
};

#endif