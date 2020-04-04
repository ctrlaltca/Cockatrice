#include "carddatabaseparser.h"

#include <QBuffer>
#include <QFile>

SetNameMap ICardDatabaseParser::sets;

void ICardDatabaseParser::clearSetlist()
{
    sets.clear();
}

CardSetPtr ICardDatabaseParser::internalAddSet(const QString &setName,
                                               const QString &longName,
                                               const QString &setType,
                                               const QDate &releaseDate)
{
    if (sets.contains(setName)) {
        return sets.value(setName);
    }

    CardSetPtr newSet = CardSet::newInstance(setName);
    newSet->setLongName(longName);
    newSet->setSetType(setType);
    newSet->setReleaseDate(releaseDate);

    sets.insert(setName, newSet);
    emit addSet(newSet);
    return newSet;
}

bool ICardDatabaseParser::saveToFile(SetNameMap sets,
                                     CardNameMap cards,
                                     const QString &fileName,
                                     const QString &sourceUrl,
                                     const QString &sourceVersion)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    return internalSaveToIODevice(sets, cards, file, sourceUrl, sourceVersion);
}

bool ICardDatabaseParser::saveToByteArray(SetNameMap sets,
                                          CardNameMap cards,
                                          QByteArray &byteArray,
                                          const QString &sourceUrl,
                                          const QString &sourceVersion)
{
    byteArray.clear();
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::WriteOnly);
    return internalSaveToIODevice(sets, cards, buffer, sourceUrl, sourceVersion);
}
