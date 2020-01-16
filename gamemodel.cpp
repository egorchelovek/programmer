#include "gamemodel.h"

GameModel::GameModel()
{
    init();
}

GameModel::~GameModel()
{
    player.skills.clear();
    continuum.clear();
}

void GameModel::init()
{
    /*
     * init player
     */
    player.mood = 1;
    player.time = 876000; // 100 years in hours
    player.money = 100.00; // $
    player.health = 100;
    player.intellect = 100;
    player.reputation = 0;
    player.skills = {};

    /*
     * init continuum
     */
    QFile file(QString(":/res/gamedata.txt"));
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            // get fields
            QStringList fields = in.readLine().split(",");

            // parse skills
            QMap<QString, uint> skills;
            if(fields[8].trimmed() != "null")
            {
                foreach(QString str, fields[8].split(" "))
                {
                    if(str != "")
                    {
                        QStringList strList = str.split(":");
                        if(strList.size() > 1)
                            skills[strList[0]] = strList[1].toUInt();
                    }
                }
            }

            // setup parameters
            Parameters pars = {
                        fields[2].toInt(),
                        fields[3].toFloat(),
                        fields[4].toInt(),
                        fields[5].toInt(),
                        fields[6].toInt(),
                        fields[7].toInt(),
                        skills};

            // wrap phenomena
            QStringList listLabel = fields[1].split(".");

            QString formatedLabel = listLabel.size() > 1? listLabel[1].trimmed()
                    + " (" + listLabel[0].trimmed() + ")" : fields[1];

            Phenomenon phen = { fields[0], formatedLabel, pars, false };

            // expand continuum
            continuum.append(phen);
        }
    }

    /*
     * start game
     */
    pause = false;
}

void GameModel::act(Phenomenon event)
{
    /*
     * update the parameters
     */
    player.time -= event.change.time * 3 / (float(player.intellect) / 100.0);
    player.money += event.change.money;
    player.health += event.change.health;  
    player.intellect += event.change.intellect;
    player.mood += event.change.mood;
    player.reputation += event.change.reputation;

    /*
     * update skills
     */
    foreach(QString skill, event.change.skills.keys())
    {
        if(skill == "*")
        {
            foreach(QString all, player.skills.keys())
            {
                player.skills[all] += 1;
            }
        }
        else if(skill == ".")
        {
            int rand = qrand() % player.skills.keys().size();
            QString any = player.skills.keys()[rand];
            player.skills[any] += 1;
        }
        else
        {
            player.skills[skill] = event.change.skills[skill];
        }
    }

    /*
     * check parameters values
     */
    bool gameNotEnd = true;
    if(player.health > 100)
    {
        player.health = 100;
    }
    else if(player.health <= 0)
    {
        gameNotEnd = false;
    }
    if(player.intellect > 230) player.intellect = 230;

    /*
     * send signals to UI
     */
    if(gameNotEnd)
    {
        emit actComplete(event);
    }
    else
    {
        emit gameEnd(getScore());
    }

}

bool GameModel::act(QString type, QString label, bool onlyNew)
{
    bool actSucceed = false;

    // get first phenomen by condition
    for(int i = 0; i < continuum.size(); i++)
    {
        if(continuum[i].type == type && (continuum[i].label == label || label == ""))
        {
            if((onlyNew == true && continuum[i].was == false) || !onlyNew)
            {
                act(continuum[i]);
                continuum[i].was = true;
                actSucceed = true;
                break;
            }
        }
    }

    return actSucceed;
}

void GameModel::loop()
{
    qsrand(42);

    // main game cycle
    while(player.time >= 0)
    {
        // simple delay for 2 ms
        QThread::msleep(2);

        // provide game pause
        if(pause)
            continue;

        //

        // tick time
        player.time -= 2 / getSpeed();
        player.money -= 0.01 / getSpeed();

        // generate accident
        int rand = qrand() % 1000;
        if(rand <= 1)
        {
            act("Случай", "Болезнь");
        }
        else if(rand <= 2)
        {
            act("Случай", "Стресс");
        }
        else if(rand <= 3)
        {
            act("Случай", "Праздник");
        }

        // send signal to UI
        emit timeUpdate();
    }

    player.time = 0;

    emit gameEnd(getScore());
}

float GameModel::getSpeed()
{
    return float(player.intellect)/100.0;
}

int GameModel::getScore()
{
    return player.mood +
            player.money +
            player.health +
            player.intellect +
            player.reputation +
            player.skills.size();
}
