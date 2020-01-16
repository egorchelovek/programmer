#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QMessageBox>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QPixmap>

#include <QDebug>

#include "gamemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    GameModel gm; // game logic

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    /*
     * GameModel process slots
     */
    void processActComplete(Phenomenon event);

    void processTimeUpdate();

    void processGameEnd(int gameScore);

    /*
     *  UI elements process slots
     */
    void on_readBookButton_clicked();

    void on_learnCourseButton_clicked();

    void on_getAnswerButton_clicked();

    void on_takeProjectButton_clicked();

    void on_findJobButton_clicked();

    void on_doJobButton_clicked();

    void on_chooseRestButton_clicked();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;

    /*
     * UI inner methods
     */
    void init();

    void runStandartAct(QString type, bool onlyNew = true);

    void updatePlayerInfo();

    void updateSkillsList();

    void updateJobList();

    void updateRestList();

    void showEventDialog(Phenomenon event);

    void showDialog(QString message, QString text);

    void showImageDialog(QString message, QString text);

    QString unfoldProjectLabel(QString text);
};
#endif // MAINWINDOW_H
