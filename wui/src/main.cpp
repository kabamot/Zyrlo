/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // This is required for tesseract
    qputenv("LC_ALL", "C");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
