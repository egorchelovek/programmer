#include "mainwindow.h"

#include <QApplication>

QString getCustomStyle()
{
    QFile file(":/style.qss");
    file.open(QFile::ReadOnly);
    return QLatin1String(file.readAll());

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); a.setStyleSheet(getCustomStyle());
    MainWindow w;
    w.show();
    return a.exec();
}
