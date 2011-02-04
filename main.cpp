/**
*
* \author Vladimír Matěna
*
* Contact vlada.matena@gmail.com
*
*/

#include <QtGui/QApplication>
#include "hddtest.h"
#include <qpushbutton.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// launch main app
	HDDTest w;
    w.show();
    return a.exec();
}
