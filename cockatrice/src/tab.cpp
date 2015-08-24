#include "tab.h"
#include "cardinfowidget.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
Tab::Tab(TabSupervisor *_tabSupervisor, QWidget *parent)
    : QMainWindow(parent), tabSupervisor(_tabSupervisor), contentsChanged(false), infoPopup(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void Tab::showCardInfoPopup(const QPoint &pos, const QString &cardName, const QString &cardHash)
{
    if (infoPopup) {
        infoPopup->deleteLater();
    }
    currentCardName = cardName;
    currentCardHash = cardHash;
    infoPopup = new CardInfoWidget(
        cardName, cardHash, 0,
        Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    infoPopup->setAttribute(Qt::WA_TransparentForMouseEvents);
    QRect screenRect = qApp->desktop()->screenGeometry(this);
    infoPopup->move(qMax(screenRect.left(), qMin(pos.x() - infoPopup->width() / 2,
                                                 screenRect.left() + screenRect.width() - infoPopup->width())),
                    qMax(screenRect.top(), qMin(pos.y() - infoPopup->height() / 2,
                                                screenRect.top() + screenRect.height() - infoPopup->height())));
    infoPopup->show();
}

void Tab::deleteCardInfoPopup(const QString &cardName, const QString & cardHash)
{
    if (infoPopup) {
        if ((currentCardName == cardName && currentCardHash == cardHash) || (cardName == "_")) {
            infoPopup->deleteLater();
            infoPopup = 0;
        }
    }
}
