#include "stdafx.h"
#include "MainWindow.h"
#include <QtGlobal>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	qInstallMessageHandler(outputMessage); //ע��MsgHandler�ص�����

	PathPlanGui w;
	w.show();
	return a.exec();
}
