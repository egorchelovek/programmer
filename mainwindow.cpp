#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    // show get player name dialog
    bool ok;
    QString text = QInputDialog::getText(this,
                                           tr("Как вас зовут?"),
                                           tr("Ваше имя:"),
                                           QLineEdit::Normal,
                                           QString("anon"),
                                           &ok);

    // show greetings
    if(ok && !text.isEmpty())
        ui->labelName->setText(tr("Привет, ") + text + "!");

    // get player name age
    int age = QInputDialog::getInt(this,
                                 tr("Сколько вам лет?"),
                                 tr("Ваш возраст:"),
                                 23, 7, 42, 1, &ok);

    // setup initial player time
    int yearHours = 24 * 365;
    gm.player.time = (42 - age) * yearHours; // 100 years minus age in hours
    if(gm.player.time < 25 * yearHours)
    {
        gm.player.time = 25 * yearHours;
    }
    ui->progressTime->setMaximum(gm.player.time);

    // update UI
    updatePlayerInfo();

    // clear UI list selection
    ui->skillsList->clearSelection();
    ui->jobList->clearSelection();
    ui->restList->clearSelection();

    // connect game model output signals with UI slots
    connect(&gm, SIGNAL(actComplete(Phenomenon)), this, SLOT(processActComplete(Phenomenon)));
    connect(&gm, SIGNAL(timeUpdate()), this, SLOT(processTimeUpdate()));
    connect(&gm, SIGNAL(gameEnd(int)), this, SLOT(processGameEnd(int)));

    // run game loop
    QFuture<void> future = QtConcurrent::run([=](){
        gm.loop();
    });
}

void MainWindow::runStandartAct(QString type, bool onlyNew)
{
    if(!gm.act(type, "", onlyNew))
    {
        showDialog("Сообщение","Ничего интересного!");
    }
}

void MainWindow::processActComplete(Phenomenon event)
{
    updatePlayerInfo();
    showEventDialog(event);
}

void MainWindow::processTimeUpdate()
{
    ui->progressTime->setValue(gm.player.time);
    ui->progressTime->update();

    ui->labelMoney->setText(QString("Деньги: ") + QString::number(gm.player.money,'f',2) + "$");

    updateRestList();
}

void MainWindow::processGameEnd(int gameScore)
{
    showDialog("Конец игры", QString("Ваш счёт: ") + QString::number(gameScore));

    QCoreApplication::exit();
}

void MainWindow::updatePlayerInfo()
{
    // update labels
    QString moodText = "Нейтральное";
    if(gm.player.mood > 0)
    {
        moodText = "Хорошее";
    }
    else if(gm.player.mood < 0)
    {
        moodText = "Плохое";
    }
    ui->labelMood->setText(QString("Настроение: ") + moodText);

    ui->labelMoney->setText(QString("Деньги: ") + QString::number(gm.player.money,'f',2) + "$");

    QString healthText = "Плохое";
    if(gm.player.health > 66)
    {
        healthText = "Отличное";
    }
    else if(gm.player.health > 33)
    {
        healthText = "Нормальное";
    }
    ui->labelHealth->setText(QString("Здоровье: ") + healthText);

    ui->labelIntellect->setText(QString("Интеллект (IQ): ") + QString::number(gm.player.intellect));

    QString textReputation = "О вас никто не знает";
    if(gm.player.reputation > 10)
    {
        textReputation = "Вам доверяют";
    }
    else if(gm.player.reputation > 1)
    {
        textReputation = "О вас знают";
    }
    else if(gm.player.reputation < 0)
    {
        textReputation = "Вам не доверяют";
    }
    ui->labelReputation->setText(QString("Репутация: ") + textReputation);

    // update progress
    ui->progressTime->setValue(gm.player.time);

    // update lists
    updateSkillsList();
    updateJobList();
    updateRestList();
}

void MainWindow::updateSkillsList()
{
    // prepare skills list
    QStringList keys = gm.player.skills.keys();
    QList<uint> values = gm.player.skills.values();
    QStringList skills;
    for(int i = 0; i < keys.size(); i++)
    {
        QString level = "Junior";
        if(values[i] >= 3 && values[i] <= 6)
        {
            level = "Middle";
        }
        else if(values[i] >= 7)
        {
            level = "Senior";
        }

        skills.append(keys[i] + "\t" + level);
    }
    skills.sort();

    // update skills list
    ui->skillsList->clear();
    ui->skillsList->addItems(skills);
}

void MainWindow::updateJobList()
{
    ui->jobList->clear();
    foreach(Phenomenon phen, gm.continuum)
    {
        if(phen.type == "Работа")
        {
            ui->jobList->addItem(phen.label);

            // modify item display settings
            QListWidgetItem* item = ui->jobList->item(ui->jobList->count() - 1);
            if(item != nullptr)
            {
            if(phen.was == true)
            {
                // hide job that was already done
                item->setHidden(true);
            }
            else
            {
                // check is job allowed
                bool jobAllowed = false;

                if(gm.player.reputation >= phen.change.reputation)
                {
                    QString mainSkill = phen.change.skills.firstKey();
                    if(gm.player.skills.contains(mainSkill))
                    {
                        if(gm.player.skills[mainSkill] >= phen.change.skills[mainSkill])
                        {
                            jobAllowed = true;
                        }
                    }
                }

                // set color gray for not allowed job
                if(!jobAllowed)
                    item->setTextColor(QColor("gray"));
            }
            }
        }
    }
}

void MainWindow::updateRestList()
{
    ui->restList->clear();

    foreach(Phenomenon phen, gm.continuum)
    {
        if(phen.type == "Место")
        {
            ui->restList->addItem(phen.label);

            if(gm.player.money < phen.change.money*(-1))
            {
                ui->restList->item(ui->restList->count() - 1)->setTextColor(QColor("gray"));
            }
        }
    }

}

void MainWindow::showEventDialog(Phenomenon event)
{
    QString message = "Сообщение";
    if(event.type == "Книга")
    {
        message = "Вы прочитали книгу";
    }
    else if(event.type == "Курс")
    {
        message = "Вы прошли курс";
    }
    else if(event.type == "Вопрос")
    {
        message = "Вы исследовали вопрос";
    }
    else if(event.type == "Работа")
    {
        message = "Вы выполнили работу";

        ui->followProjectLabel->setText(unfoldProjectLabel(event.label));
    }
    else if(event.type == "Проект")
    {
        message = "Вы выполнили проект";

        ui->followProjectLabel->setText(unfoldProjectLabel(event.label));
    }
    else if(event.type == "Место")
    {
        message = "Вы отдохнули в месте";
    }
    else if(event.type == "Событие")
    {
        message = "Непредвиденное обстоятельство";
    }

    if(event.type == "Место")
    {
        showImageDialog(message, event.label);
    }
    else
    {
        showDialog(message, event.label);
    }
}

QString MainWindow::unfoldProjectLabel(QString label)
{
    QStringList stringList = label.split(" ");
    return "Последний проект: " +
            stringList[stringList.size() - 2].replace("(","").replace(")","") +
            " " + stringList[stringList.size()-1];
}

void MainWindow::showDialog(QString message, QString text)
{
    QMessageBox dialog;
    dialog.setWindowTitle(message);
    dialog.setText(text);
    dialog.exec();
}

void MainWindow::showImageDialog(QString message, QString text)
{
    QString place = text.split(" ")[0].trimmed();
    QMessageBox dialog;
    QPixmap pix(":/res/images/rest/"+place+".jpeg");
    int scaleFactor = 800;
    if(pix.size().width() > pix.size().height())
    {
        scaleFactor = (scaleFactor > pix.size().width()? pix.size().width() : scaleFactor);
        dialog.setIconPixmap(pix.scaledToWidth(scaleFactor));
    }
    else
    {
        scaleFactor = (scaleFactor > pix.size().height()? pix.size().height() : scaleFactor);
        dialog.setIconPixmap(pix.scaledToHeight(scaleFactor));
    }
    dialog.setWindowTitle(message);
    dialog.setText(text);
    dialog.exec();
}

void MainWindow::on_readBookButton_clicked()
{
    runStandartAct("Книга");
}

void MainWindow::on_learnCourseButton_clicked()
{
    runStandartAct("Курс");
}

void MainWindow::on_getAnswerButton_clicked()
{
    runStandartAct("Вопрос", false);
}

void MainWindow::on_takeProjectButton_clicked()
{
    if(gm.player.skills.size() > 0)
    {
        runStandartAct("Проект", false);
    }
    else
    {
        showDialog("Сообщение","Вы не можете выполнить проект. "
                               "\n Вы ничего не умеете.");
    }
}

void MainWindow::on_findJobButton_clicked()
{
    updateJobList();

    ui->stackedWidget->setCurrentIndex(1);

}

void MainWindow::on_doJobButton_clicked()
{
    if(ui->jobList->selectedItems().size() > 0)
    {
        if(ui->jobList->selectedItems()[0]->textColor() == QColor("gray"))
        {
            showDialog("Сообщение", "Вы не можете сделать эту работу. "
                                    "\n У вас недостаточно навыка или репутации.");
        }
        else
        {
            gm.act("Работа", ui->jobList->selectedItems()[0]->text());
        }
    }

    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_chooseRestButton_clicked()
{
    if(ui->restList->selectedItems().size() > 0)
    {

        if(ui->restList->selectedItems()[0]->textColor() != QColor("gray"))
        {
            gm.act("Место", ui->restList->selectedItems()[0]->text());
        }
        else
        {
            showDialog("Сообщение", "У вас недостаточно денег.");
        }
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(0);

    if(ui->tabWidget->currentWidget() != ui->tab_main)
    {
        gm.pause = true;
    }
    else
    {
        gm.pause = false;
    }
}
