#include "oracleimporter.h"
#include "carddbparser/cockatricexml3.h"

#include <QDebug>
#include <QtWidgets>
#include <climits>

#include "qt-json/json.h"

OracleImporter::OracleImporter(const QString &_dataDir, QObject *parent) : CardDatabase(parent), dataDir(_dataDir)
{
}

bool OracleImporter::readSetsFromByteArray(const QByteArray &data)
{
    QList<SetToDownload> newSetList;

    bool ok;
    setsMap = QtJson::Json::parse(QString(data), ok).toMap();
    if (!ok) {
        qDebug() << "error: QtJson::Json::parse()";
        return 0;
    }

    QListIterator<QVariant> it(setsMap.values());
    QVariantMap map;

    QString edition;
    QString editionLong;
    QVariant editionCards;
    QString setType;
    QDate releaseDate;

    while (it.hasNext()) {
        map = it.next().toMap();
        edition = map.value("code").toString();
        editionLong = map.value("name").toString();
        editionCards = map.value("cards");
        setType = map.value("type").toString();
        // capitalize set type
        if (setType.length() > 0)
            setType[0] = setType[0].toUpper();
        releaseDate = map.value("releaseDate").toDate();

        newSetList.append(SetToDownload(edition, editionLong, editionCards, setType, releaseDate));
    }

    qSort(newSetList);

    if (newSetList.isEmpty())
        return false;
    allSets = newSetList;
    return true;
}

CardInfoPtr OracleImporter::addCard(CardSetPtr set,
    const QString &cardHash,
    QString cardName,
    bool isToken,
    QVariantHash properties,
    const QString &cardType,
    const QString &cardText,
    const QString &picUrl,
    const QList<CardRelation *> &relatedCards,
    const QList<CardRelation *> &reverseRelatedCards,
    bool upsideDown
) {
    // qDebug() << "addcard" << cardHash << cardName << setName;

    // Workaround for card name weirdness
    cardName = cardName.replace("Æ", "AE");
    cardName = cardName.replace("’", "'");

    // Remove {} around mana costs, except if it's split cost
    if(properties.contains("manacost"))
    {
        QString cardCost = properties.value("manacost").toString();
        QStringList symbols = cardCost.split("}");
        QString formattedCardCost = QString();
        for (QString symbol : symbols) {
            if (symbol.contains(QRegExp("[0-9WUBGRP]/[0-9WUBGRP]"))) {
                symbol.append("}");
            } else {
                symbol.remove(QChar('{'));
            }
            formattedCardCost.append(symbol);
        }

        properties.insert("manacost", formattedCardCost);
    }

    // detect mana generator artifacts
    bool mArtifact = false;
    if (cardType.endsWith("Artifact")) {
        QStringList cardTextRows = cardText.split("\n");
        for (int i = 0; i < cardTextRows.size(); ++i) {
            cardTextRows[i].remove(QRegularExpression("\\\".*?\\\""));
            if (cardTextRows[i].contains("{T}") && cardTextRows[i].contains("to your mana pool")) {
                mArtifact = true;
            }
        }
    }

    // detect cards that enter the field tapped
    bool cipt =
        cardText.contains("Hideaway") || (cardText.contains(cardName + " enters the battlefield tapped") &&
                                          !cardText.contains(cardName + " enters the battlefield tapped unless"));

    // insert the card and its properties
    CardInfoPtr card = CardInfo::newInstance(set, cardHash, cardName, isToken, properties, cardType, cardText, picUrl, relatedCards, reverseRelatedCards, upsideDown, cipt);

    int tableRow = 1;
    QString mainCardType = card->getMainCardType();
    if ((mainCardType == "Land") || mArtifact)
        tableRow = 0;
    else if ((mainCardType == "Sorcery") || (mainCardType == "Instant"))
        tableRow = 3;
    else if (mainCardType == "Creature")
        tableRow = 2;
    card->setTableRow(tableRow);

    cards.insert(cardName, card);
    return card;
}

QString OracleImporter::extractColors(const QStringList & in)
{
    QString out;
    foreach(QString c, in)
    {
        if (c == "White")
            out.append("W");
        else if (c == "Blue")
            out.append("U");
        else if (c == "Black")
            out.append("B");
        else if (c == "Red")
            out.append("R");
        else if (c == "Green")
            out.append("G");
        else
            qDebug() << "error: unknown color:" << c;
    }
    return out;
}

int OracleImporter::importTextSpoiler(CardSetPtr set, const QVariant &data)
{
    int cards = 0;

    QListIterator<QVariant> it(data.toList());
    QVariantMap map;
    QVariantHash properties;
    QString layout, cardHash, cardName, cardType, cardText, picUrl;
    QList<CardRelation *> relatedCards;
    QList<CardRelation *> reverseRelatedCards; // dummy
    bool upsideDown;

    // split cards
    QMap<QString, QVariantMap> splitCards;
    QString splitSeparator = QString(" // ");
    QString splitSeparator2 = QString("\n\n---\n\n");

    while (it.hasNext()) {
        map = it.next().toMap();

        layout = map.value("layout").toString();

        // don't import tokens from the json file
        if (layout == "token")
            continue;

        if (layout == "split" || layout == "aftermath") {
            // Enqueue split card for later handling
            QString splitCardIdentifier = set->getShortName() + "_" + map.value("names").toStringList().join("_");
            splitCards.insertMulti(splitCardIdentifier, map);
            continue;
        }

        cardHash.clear();
        cardName.clear();
        cardType.clear();
        cardText.clear();
        properties.clear();
        relatedCards.clear();
        upsideDown = false;

        foreach(QString key, map.keys())
        {
            if(key == "id")
                cardHash = map.value("id").toString();
            else if(key == "name")
                cardName = map.value("name").toString();
            else if(key == "type")
                cardType = map.value("type").toString();
            else if(key == "text")
                cardText = map.value("text").toString();
            else if(key == "names")
            {
                foreach (const QString &name, map.value("names").toStringList()) {
                    if (name != cardName)
                    {
                        relatedCards.append(new CardRelation(name, true));
                    }
                }
            }
            // complex properties
            else if(key == "multiverseid")
                properties.insert("muid", map.value(key).toString());
            else if(key == "loyalty")
                properties.insert("loyalty", map.value(key).toString());
            else if(key == "cmc")
                properties.insert("cmc", map.value(key).toString());
            else if(key == "number")
                properties.insert("number", map.value(key).toString());
            else if(key == "manaCost")
                properties.insert("manacost", map.value(key).toString());
            else if(key == "rarity")
                properties.insert("rarity", map.value(key).toString());
            // complex properties with special handling
            else if(key == "power" || key == "toughness")
                properties.insert("pt", map.value("power").toString() + QString('/') + map.value("toughness").toString());
            else if(key == "colors")
                properties.insert("colors", extractColors(map.value("colors").toStringList()));
        }

        // special handling for flip cards
        if(layout == "flip")
        {
            QStringList cardNames = map.value("names", QStringList()).toStringList();
            upsideDown = (cardNames.indexOf(cardName) > 0);
        } else {
            upsideDown = false;
        }

        addCard(set, cardHash, cardName, false, properties, cardType, cardText, picUrl, relatedCards, reverseRelatedCards, upsideDown);
        cards++;
    }

    // split cards handling - get all unique card muids
    QStringList allSplits = splitCards.uniqueKeys();
    foreach(QString splitCardIdentifier, allSplits)
    {
        // get all cards for this specific muid
        QList<QVariantMap> maps = splitCards.values(splitCardIdentifier);
        QStringList names;
        // now, reorder the cards using the ordered list of names
        QMap<int, QVariantMap> orderedMaps;
        foreach(QVariantMap map, maps)
        {
            if(names.isEmpty())
                names = map.value("names", QStringList()).toStringList();
            QString name = map.value("name").toString();
            int index = names.indexOf(name);
            orderedMaps.insertMulti(index, map);
        }

        // clean variables
        cardHash.clear();
        cardName.clear();
        cardType.clear();
        cardText.clear();
        properties.clear();
        relatedCards.clear();
        upsideDown = false;

        // loop cards and merge their contents
        foreach(QVariantMap map, orderedMaps.values())
        {
            foreach(QString key, map.keys())
            {
                // use the cardHash from the first card only
                if(key == "id" && cardHash.isEmpty())
                    cardHash = map.value("id").toString();
                else if(key == "name") {
                    if(!cardName.isEmpty())
                        cardName += splitSeparator;
                    cardName += map.value("name").toString();
                } else if(key == "type") {
                    if(!cardType.isEmpty())
                        cardType += splitSeparator;
                    cardType += map.value("type").toString();
                } else if(key == "text") {
                    if(!cardText.isEmpty())
                        cardText += splitSeparator2;
                    cardText += map.value("text").toString();
                }
                // complex properties
                else if(key == "multiverseid" && !properties.contains("muid")) {
                    // we use the muid from the firt card (they are supposed to be the same anyway)
                    properties.insert("muid", map.value(key).toString());
                } else if(key == "loyalty") {
                    QString currentValue = properties.value("loyalty", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value(key).toString();
                    properties.insert("loyalty", currentValue);
                } else if(key == "cmc") {
                    QString currentValue = properties.value("cmc", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value(key).toString();
                    properties.insert("cmc", currentValue);
                } else if(key == "manaCost") {
                    QString currentValue = properties.value("manacost", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value(key).toString();
                    properties.insert("manacost", currentValue);
                } else if(key == "number") {
                    QString currentValue = properties.value("number", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value(key).toString();
                    properties.insert("number", currentValue);
                } else if(key == "rarity") {
                    QString currentValue = properties.value("rarity", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value(key).toString();
                    properties.insert("rarity", currentValue);
                } else if(key == "power" || key == "toughness") {
                    QString currentValue = properties.value("pt", QString("")).toString();
                    if(!currentValue.isEmpty())
                        currentValue += splitSeparator;
                    currentValue += map.value("power").toString() + QString('/') + map.value("toughness").toString();
                    properties.insert("pt", currentValue);
                // complex properties with special handling
                } else if(key == "colors") {
                    QStringList currentValue = properties.value("colors", QStringList()).toStringList();
                    currentValue += extractColors(map.value("colors").toStringList());
                    currentValue.removeDuplicates();
                    if (currentValue.length() > 1) {
                        sortColors(currentValue);
                    }
                    properties.insert("colors", currentValue.join(""));
                }
            }
        }

        // Fortunately, there are no split cards that flip, transform or meld.
        relatedCards = QList<CardRelation *>();
        reverseRelatedCards = QList<CardRelation *>();
        upsideDown = false;

        // add the card
        addCard(set, cardHash, cardName, false, properties, cardType, cardText, picUrl, relatedCards, reverseRelatedCards, upsideDown);
        cards++;
    }

    return cards;
}

void OracleImporter::sortColors(QStringList &colors)
{
    const QHash<QString, unsigned int> colorOrder{{"W", 0}, {"U", 1}, {"B", 2}, {"R", 3}, {"G", 4}};
    std::sort(colors.begin(), colors.end(), [&colorOrder](const QString a, const QString b) {
        return colorOrder.value(a, INT_MAX) < colorOrder.value(b, INT_MAX);
    });
}

int OracleImporter::startImport()
{
    clear();

    int setCards = 0, setIndex = 0;
    QListIterator<SetToDownload> it(allSets);
    const SetToDownload *curSet;

    // add an empty set for tokens
    CardSetPtr tokenSet = CardSet::newInstance(TOKENS_SETNAME, tr("Dummy set containing tokens"), "Tokens");
    sets.insert(TOKENS_SETNAME, tokenSet);

    while (it.hasNext()) {
        curSet = &it.next();
        CardSetPtr set = CardSet::newInstance(curSet->getShortName(), curSet->getLongName(), curSet->getSetType(),
                                              curSet->getReleaseDate());
        if (!sets.contains(set->getShortName()))
            sets.insert(set->getShortName(), set);

        int setCardsHere = importTextSpoiler(set, curSet->getCards());

        ++setIndex;

        emit setIndexChanged(setCardsHere, setIndex, curSet->getLongName());
    }

    emit setIndexChanged(setCards, setIndex, QString());

    // total number of sets
    return setIndex;
}

bool OracleImporter::saveToFile(const QString &fileName)
{
    CockatriceXml3Parser parser;
    return parser.saveToFile(sets, cards, fileName);
}