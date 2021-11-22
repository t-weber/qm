/**
 * program entry point
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include "qm_gui.h"
#include "helpers.h"

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
		//app->setApplicationDisplayName("Quantum Algorithms Editor");
		app->setApplicationVersion("0.4");
		set_locales();

		// main window
		auto qm = std::make_unique<QmWnd>();

		// ressources
		Resources& res = qm->GetResources();
		fs::path appdir = app->applicationDirPath().toStdString();
		res.AddPath(appdir);
		res.AddPath(appdir / "res");
		res.AddPath(appdir / ".." / "res");

		// setup main window gui
		qm->SetupGUI();
		qm->FileNew();

		// show the main window
		show_dialog(qm.get());
		return app->exec();
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	return -1;
}
