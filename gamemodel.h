#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QString>
#include <QMap>
#include <QTime>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QThread>

/*
 * Player state
 */
struct Parameters
{
    int time; // hours
    float money; // $
    int health;
    int intellect;
    int mood;
    int reputation;
    QMap<QString, uint> skills;
};

/*
 *  Sources of changes
 */
struct Phenomenon
{
    QString type; // what?
    QString label; // what exactly?
    Parameters change; // how it changes parameters?
    bool was; // it is already happend?
};

/*
 * Game logic
 */
class GameModel : public QObject
{
    Q_OBJECT

    void init(); // setup game

    void act(Phenomenon event); // run single event

    float getSpeed(); // defined by intellect

    int getScore(); // calculate game score

public:
    GameModel();
    ~GameModel();

    Parameters player; // player state

    QList<Phenomenon> continuum; // all possible events

    bool act(QString type, QString label = "", bool onlyNew = false); // single event

    void loop(); // main game cycle

    bool pause; // change this flag to pause game

// output
signals:
    void actComplete(Phenomenon event);

    void timeUpdate();

    void gameEnd(int gameScore);
};

#endif // GAMEMODEL_H
