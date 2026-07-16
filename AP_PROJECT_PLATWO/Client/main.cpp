#include "Loginwindow.h"
#include <QFile>
#include <QString>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QApplication::setQuitOnLastWindowClosed(false);

    QFile file(":/qss/theme.qss");
    if (file.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }


    Loginwindow* w = new Loginwindow();
    w->show();

    return QApplication::exec();
}