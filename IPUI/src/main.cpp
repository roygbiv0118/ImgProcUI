#include "ipui.h"
#include <QtWidgets/QApplication>
#include <QResource>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());
	a.setWindowIcon(QIcon("../Res/Logo.ico"));

	QTranslator trans;
	bool bRes = trans.load("../Res/mx_zh.qm");
	a.installTranslator(&trans);

	IPUI w;
	w.show();
	return a.exec();
}
