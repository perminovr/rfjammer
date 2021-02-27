#include "mapplication.h"

// qt includes are not allowed before wt
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	// one MApplication object is created per each client connection
	return Wt::WRun(argc, argv, &MApplication::createApplication);
}

