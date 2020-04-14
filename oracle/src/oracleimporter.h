#ifndef ORACLEIMPORTER_H
#define ORACLEIMPORTER_H

#include <QMap>
#include <QVariant>
#include <carddatabase.h>
#include <utility>

// many users prefer not to see these sets with non english arts
// as a solution we remove the date property on these sets
// that way they will be sorted last by default
// this will cause their art to not get priority over english cards
// users will still be able to find these sets and prioritize them manually
const QStringList nonEnglishSets = {"4BB", "FBB", "PS11", "PSAL", "REN", "RIN"};

class SplitCardPart
{
public:
    SplitCardPart(int _index, const QString &_text, const QVariantHash &_properties, CardInfoPerSet setInfo);
    inline const int &getIndex() const
    {
        return index;
    }
    inline const QString &getText() const
    {
        return text;
    }
    inline const QVariantHash &getProperties() const
    {
        return properties;
    }
    inline const CardInfoPerSet &getSetInfo() const
    {
        return setInfo;
    }

private:
    int index;
    QString text;
    QVariantHash properties;
    CardInfoPerSet setInfo;
};

class OracleImporter : public CardDatabase
{
    Q_OBJECT
private:
    const QStringList mainCardTypes = {"Planeswalker", "Creature", "Land",       "Sorcery",
                                       "Instant",      "Artifact", "Enchantment"};
    QString dataDir;

    QString getMainCardType(const QStringList &typeList);
    CardInfoPtr addCard(QString name,
                        QString text,
                        bool isToken,
                        QVariantHash properties,
                        QList<CardRelation *> &relatedCards,
                        CardInfoPerSet setInfo);
signals:
    void internalParseProgress(int current, int total);
    void dataReadProgress(int bytesRead, int totalBytes);

public:
    explicit OracleImporter(const QString &_dataDir, QObject *parent = nullptr);
    QString startImport(QByteArray &data, const QString &sourceUrl, const QString & sourceVersion);
    int importCardsFromSet(CardSetPtr currentSet, const QList<QVariant> &cards, bool skipSpecialNums = true);
    const QString &getDataDir() const
    {
        return dataDir;
    }

protected:
    inline QString capitalizeSetType(QString setType);
    inline QString getStringPropertyFromMap(QVariantMap card, QString propertyName);
    void sortAndReduceColors(QString &colors);

protected slots:
    void jsonProgress(int current, int total);
};

#endif
