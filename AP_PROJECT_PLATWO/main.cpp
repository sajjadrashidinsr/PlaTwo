#include "Loginwindow.h"
#include <QFile>
#include <QString>
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Loginwindow w;
    QFile file(":/qss/theme.qss");

    if (file.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(file.readAll());
        a.setStyleSheet(styleSheet);
    }
    w.show();
    return QApplication::exec();
}
