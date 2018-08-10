#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include <QBasicMutex>
#include <QDate>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <QVariant>

class CardDatabase;
class CardInfo;
class CardSet;
class CardRelation;
class ICardDatabaseParser;

typedef QSharedPointer<CardInfo> CardInfoPtr;
typedef QSharedPointer<CardSet> CardSetPtr;

Q_DECLARE_METATYPE(CardInfoPtr)

class CardSet : public QList<CardInfoPtr>
{
private:
    QString shortName, longName;
    unsigned int sortKey;
    QDate releaseDate;
    QString setType;
    bool enabled, isknown;

public:
    explicit CardSet(const QString &_shortName = QString(),
                     const QString &_longName = QString(),
                     const QString &_setType = QString(),
                     const QDate &_releaseDate = QDate());
    static CardSetPtr newInstance(const QString &_shortName = QString(),
                                  const QString &_longName = QString(),
                                  const QString &_setType = QString(),
                                  const QDate &_releaseDate = QDate());
    QString getCorrectedShortName() const;
    QString getShortName() const
    {
        return shortName;
    }
    QString getLongName() const
    {
        return longName;
    }
    QString getSetType() const
    {
        return setType;
    }
    QDate getReleaseDate() const
    {
        return releaseDate;
    }
    void setLongName(const QString &_longName)
    {
        longName = _longName;
    }
    void setSetType(const QString &_setType)
    {
        setType = _setType;
    }
    void setReleaseDate(const QDate &_releaseDate)
    {
        releaseDate = _releaseDate;
    }

    void loadSetOptions();
    int getSortKey() const
    {
        return sortKey;
    }
    void setSortKey(unsigned int _sortKey);
    bool getEnabled() const
    {
        return enabled;
    }
    void setEnabled(bool _enabled);
    bool getIsKnown() const
    {
        return isknown;
    }
    void setIsKnown(bool _isknown);

    // Determine incomplete sets.
    bool getIsKnownIgnored() const
    {
        return longName.length() + setType.length() + releaseDate.toString().length() == 0;
    }
};

class SetList : public QList<CardSetPtr>
{
private:
    class KeyCompareFunctor;

public:
    void sortByKey();
    void guessSortKeys();
    void enableAllUnknown();
    void enableAll();
    void markAllAsKnown();
    int getEnabledSetsNum();
    int getUnknownSetsNum();
    QStringList getUnknownSetsNames();
};

class CardInfo : public QObject
{
    Q_OBJECT
private:
    CardInfoPtr smartThis;
    // The card hash
    QString hash;
    // The card name
    QString name;
    // The name without punctuation or capitalization, for better card tag name recognition.
    QString simpleName;
    // The key used to identify this card in the cache
    QString pixmapCacheKey;
    // card type
    QString cardtype;
    // card text
    QString text;
    // custom pic url if any
    QString picUrl;
    // card set
    CardSetPtr set;

    // basic card properties; common for all the variations
    QVariantHash properties;
    // the cards i'm related to
    QList<CardRelation *> relatedCards;

    // the card i'm reverse-related to
    QList<CardRelation *> reverseRelatedCards;

    // the cards thare are reverse-related to me
    QList<CardRelation *> reverseRelatedCardsToMe;

    // positioning properties; used by UI
    bool upsideDownArt;
    bool cipt;
    int tableRow;

    bool isToken;

public:
    explicit CardInfo(
        CardSetPtr _set,
        const QString &_hash = QString(),
        const QString &_name = QString(),
        bool _isToken = false,
        QVariantHash _properties = QVariantHash(),
        const QString &_cardtype = QString(),
        const QString &_text = QString(),
        const QString &_picUrl = QString(),
        const QList<CardRelation *> &_relatedCards = QList<CardRelation *>(),
        const QList<CardRelation *> &_reverseRelatedCards = QList<CardRelation *>(),
        bool _upsideDownArt = false,
        bool _cipt = false,
        int _tableRow = 0
        );
    ~CardInfo() override;

    static CardInfoPtr newInstance(
                                   CardSetPtr _set,
                                   const QString &_hash = QString(),
                                   const QString &_name = QString(),
                                   bool _isToken = false,
                                   QVariantHash _properties = QVariantHash(),
                                   const QString &_cardtype = QString(),
                                   const QString &_text = QString(),
                                   const QString &_picUrl = QString(),
                                   const QList<CardRelation *> &_relatedCards = QList<CardRelation *>(),
                                   const QList<CardRelation *> &_reverseRelatedCards = QList<CardRelation *>(),
                                   bool _upsideDownArt = false,
                                   bool _cipt = false,
                                   int _tableRow = 0);

    void setSmartPointer(CardInfoPtr _ptr)
    {
        smartThis = _ptr;
    }

    inline const QString &getHash() const
    {
        return hash;
    }
    inline const QString &getName() const
    {
        return name;
    }
    const QString &getSimpleName() const
    {
        return simpleName;
    }
    bool getIsToken() const
    {
        return isToken;
    }
    CardSetPtr getSet() const
    {
        return set;
    }
    inline const QString &getCardType() const
    {
        return cardtype;
    }
    const QString &getText() const
    {
        return text;
    }
    inline const QString getProperty(QString propertyName) const
    {
        return properties.value(propertyName).toString();
    }
    const QString &getPixmapCacheKey();
    bool getCipt() const
    {
        return cipt;
    }
    void setCardType(const QString &_cardType)
    {
        cardtype = _cardType;
        emit cardInfoChanged(smartThis);
    }
    void setText(const QString &_text)
    {
        text = _text;
        emit cardInfoChanged(smartThis);
    }
    const QList<CardRelation *> &getRelatedCards() const
    {
        return relatedCards;
    }
    const QList<CardRelation *> &getReverseRelatedCards() const
    {
        return reverseRelatedCards;
    }
    const QList<CardRelation *> &getReverseRelatedCards2Me() const
    {
        return reverseRelatedCardsToMe;
    }
    void resetReverseRelatedCards2Me();
    void addReverseRelatedCards2Me(CardRelation *cardRelation)
    {
        reverseRelatedCardsToMe.append(cardRelation);
    }
    bool getUpsideDownArt() const
    {
        return upsideDownArt;
    }
    QString getCustomPicURL() const
    {
        return picUrl;
    }
    QString getMainCardType() const;
    QString getCorrectedName() const;
    int getTableRow() const
    {
        return tableRow;
    }
    void setTableRow(int _tableRow)
    {
        tableRow = _tableRow;
    }
    void addToSet(CardSetPtr _set);
    void emitPixmapUpdated()
    {
        emit pixmapUpdated();
    }
    void setProperty(const QString &_name, const QString &_value)
    {
        properties.insert(_name, _value);
        emit cardInfoChanged(smartThis);
    }
    void setProperty(const QString &_name, const QStringList &_value)
    {
        properties.insert(_name, _value);
        emit cardInfoChanged(smartThis);
    }
    QVariantHash getProperties() const {
        return properties;
    }

    // Back-compatibiility methods. Remove ASAP
    const QString getManaCost() const
    {
        return getProperty("manacost");
    }
    const QString getCmc() const
    {
        return getProperty("cmc");
    }
    const QString getPowTough() const
    {
        return getProperty("pt");
    }
    const QString getRarity() const
    {
        return getProperty("rarity");
    }
    const QString getLoyalty() const
    {
        return getProperty("loyalty");
    }
    const QString getCollectorNumber() const
    {
        return getProperty("number");
    }
    const QStringList getColors() const
    {
        return properties.value("colors").toStringList();
    }
    int getMuId() const
    {
        return properties.value("muid").toInt();
    }
    const QChar getColorChar() const;
    void setPowTough(const QString &_powTough)
    {
        setProperty("pt", _powTough);
    }
    void setColors(const QStringList &_colors)
    {
        setProperty("colors", _colors);
    }

    /**
     * Simplify a name to have no punctuation and lowercase all letters, for
     * less strict name-matching.
     */
    static QString simplifyName(const QString &name);

signals:
    void pixmapUpdated();
    void cardInfoChanged(CardInfoPtr card);
};

enum LoadStatus
{
    Ok,
    VersionTooOld,
    Invalid,
    NotLoaded,
    FileError,
    NoCards
};

typedef QHash<QString, CardInfoPtr> CardStringMap;
typedef QHash<QString, CardSetPtr> SetNameMap;

class CardDatabase : public QObject
{
    Q_OBJECT
protected:
    /*
     * The cards, indexed by name.
     */
    CardStringMap cards;

    /**
     * The cards, indexed by their simple name.
     */
    CardStringMap simpleNameCards;

    /*
     * The cards, indexed by hash.
     */
    CardStringMap cardHashes;

    /*
     * The sets, indexed by short name.
     */
    SetNameMap sets;

    LoadStatus loadStatus;

    QVector<ICardDatabaseParser *> availableParsers;

private:
    CardInfoPtr getCardFromMap(const CardStringMap &cardMap, const QString &cardName) const;
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();

    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(), *clearDatabaseMutex = new QBasicMutex(),
                *loadFromFileMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    static const char *TOKENS_SETNAME;

    explicit CardDatabase(QObject *parent = nullptr);
    ~CardDatabase() override;
    void clear();
    void removeCard(CardInfoPtr card);
    CardInfoPtr getCard(const QString &cardName, const QString &cardHash) const;
    QList<CardInfoPtr> getCards(const QStringList &cardNames) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    CardSetPtr getSet(const QString &setName);
    QList<CardInfoPtr> getCardList() const
    {
        return cards.values();
    }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName);
    bool saveCustomTokensToFile();
    QStringList getAllColors() const;
    QStringList getAllMainCardTypes() const;
    LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }
    void enableAllUnknownSets();
    void markAllSetsAsKnown();
    void notifyEnabledSetsChanged();

public slots:
    LoadStatus loadCardDatabases();
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
private slots:
    LoadStatus loadCardDatabase(const QString &path);
signals:
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();
    void cardDatabaseEnabledSetsChanged();
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);
};

class CardRelation : public QObject
{
    Q_OBJECT
private:
    QString name;
    bool doesAttach;
    bool isCreateAllExclusion;
    bool isVariableCount;
    int defaultCount;

public:
    explicit CardRelation(const QString &_name = QString(),
                          bool _doesAttach = false,
                          bool _isCreateAllExclusion = false,
                          bool _isVariableCount = false,
                          int _defaultCount = 1);

    inline const QString &getName() const
    {
        return name;
    }
    bool getDoesAttach() const
    {
        return doesAttach;
    }
    bool getCanCreateAnother() const
    {
        return !doesAttach;
    }
    bool getIsCreateAllExclusion() const
    {
        return isCreateAllExclusion;
    }
    bool getIsVariable() const
    {
        return isVariableCount;
    }
    int getDefaultCount() const
    {
        return defaultCount;
    }
};
#endif