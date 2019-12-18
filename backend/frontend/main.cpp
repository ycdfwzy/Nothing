#include "frontend.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	frontend w;
	w.show();
	return a.exec();
}
