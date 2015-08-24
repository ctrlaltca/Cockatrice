#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <QMap>
#include <QVariant>

#include <carddatabase.h>

class SetToDownload
{
private:
    QString shortName, longName;
    QVariant cards;
    QDate releaseDate;
    QString setType;

public:
    const QString &getShortName() const
    {
        return shortName;
    }
    const QString &getLongName() const
    {
        return longName;
    }
    const QVariant &getCards() const
    {
        return cards;
    }
    const QString &getSetType() const
    {
        return setType;
    }
    const QDate &getReleaseDate() const
    {
        return releaseDate;
    }
    SetToDownload(const QString &_shortName,
                  const QString &_longName,
                  const QVariant &_cards,
                  const QString &_setType = QString(),
                  const QDate &_releaseDate = QDate())
        : shortName(_shortName), longName(_longName), cards(_cards), releaseDate(_releaseDate), setType(_setType)
    {
    }
    bool operator<(const SetToDownload &set) const
    {
        return longName.compare(set.longName, Qt::CaseInsensitive) < 0;
    }
};

class OracleImporter : public CardDatabase
{
    Q_OBJECT
private:
    QList<SetToDownload> allSets;
    QVariantMap setsMap;
    QString dataDir;
    CardInfoPtr addCard(
        CardSetPtr set,
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
    );
signals:
    void setIndexChanged(int cardsImported, int setIndex, const QString &setName);
    void dataReadProgress(int bytesRead, int totalBytes);

public:
    OracleImporter(const QString &_dataDir, QObject *parent = 0);
    bool readSetsFromByteArray(const QByteArray &data);
    int startImport();
    bool saveToFile(const QString &fileName);
    int importTextSpoiler(CardSetPtr set, const QVariant &data);
    QList<SetToDownload> &getSets()
    {
        return allSets;
    }
    const QString &getDataDir() const
    {
        return dataDir;
    }

protected:
    QString extractColors(const QStringList & in);
    void sortColors(QStringList &colors);
};

#endif
