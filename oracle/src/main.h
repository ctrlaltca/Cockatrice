#ifndef MAIN_H
#define MAIN_H

class QTranslator;
class QString;

extern QTranslator *translator;
extern const QString translationPrefix;
extern QString translationPath;
extern bool updateCards, updateTokens, updateSpoilers;

void installNewTranslator();

#endif
