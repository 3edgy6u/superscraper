#include "ascraper.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ascraper w;
    w.show();

    return a.exec();
}
