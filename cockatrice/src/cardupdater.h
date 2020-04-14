#ifndef CARDUPDATER_H
#define CARDUPDATER_H

#include <QProcess>
#include <QWidget>

class QSharedMemory;

class CardUpdater : public QWidget
{
    Q_OBJECT
public:
    explicit CardUpdater(QWidget *parent = nullptr);
    ~CardUpdater() {}

private:
    // TODO: add a preference item to choose updater name for other games
    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    };
    const QString getUpdaterBinaryPath();
    const QStringList getUpdaterParameters(bool checkOnly);

    static QProcess *cardUpdateProcess;
    QSharedMemory *shm;

public slots:
    void actCheckCardUpdates(bool checkOnly);
private slots:
    void cardUpdateError(QProcess::ProcessError err);
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cardCheckFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

#endif
