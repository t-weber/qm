/**
 * program entry point
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include "qm_gui.h"
#include "resources.h"

#include <QtCore/QLocale>
#include <QtWidgets/QApplication>

#include <locale>
#include <string>
#include <iostream>


static inline void set_locales()
{
	std::ios_base::sync_with_stdio(false);

	::setlocale(LC_ALL, "C");
	std::locale::global(std::locale("C"));
	QLocale::setDefault(QLocale::C);
}


int main(int argc, char** argv)
{
	try
	{
		// application
		auto app = std::make_unique<QApplication>(argc, argv);
		app->setOrganizationName("tw");
		app->setApplicationName("qm");
		set_locales();

		// resources
		Resources res;
		fs::path appdir = app->applicationDirPath().toStdString();
		res.AddPath(appdir);
		res.AddPath(appdir / "res");

		// main window
		auto qm = std::make_unique<QmWnd>();
		qm->show();

		return app->exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return -1;
}