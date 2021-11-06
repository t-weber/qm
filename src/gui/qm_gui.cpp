/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include "qm_gui.h"

#include <QtCore/QSettings>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStyleFactory>
#include <QtSvg/QSvgGenerator>
#if QT_VERSION >= 0x060000
	#include <QtGui/QActionGroup>
#else
	#include <QtWidgets/QActionGroup>
#endif

#include <fstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace ptree = boost::property_tree;


#define GUI_THEME_UNSET   "Unset"


// ----------------------------------------------------------------------------
// main window
// ----------------------------------------------------------------------------
QmWnd::QmWnd(QWidget* pParent)
	: QMainWindow{pParent},
		m_scene{new QmScene{this}},
		m_view{new QmView{m_scene.get(), this}},
		m_statusLabel{std::make_shared<QLabel>(this)}
{
	// ------------------------------------------------------------------------
	// restore settings
	QSettings settings{this};

	if(settings.contains("wnd_geo"))
	{
		QByteArray arr{settings.value("wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
	else
	{
		resize(1024, 768);
	}

	if(settings.contains("wnd_state"))
	{
		QByteArray arr{settings.value("wnd_state").toByteArray()};
		this->restoreState(arr);
	}

	if(settings.contains("wnd_theme"))
		m_gui_theme = settings.value("wnd_theme").toString();

	if(settings.contains("wnd_native"))
		m_gui_native = settings.value("wnd_native").toBool();

	if(settings.contains("file_recent"))
		m_recent.SetRecentFiles(settings.value("file_recent").toStringList());

	if(settings.contains("file_recent_dir"))
		m_recent.SetRecentDir(settings.value("file_recent_dir").toString());
	// ------------------------------------------------------------------------


	setWindowTitle("QM");
	setCentralWidget(m_view.get());

	m_properties = std::make_shared<DockWidgetWrapper<ComponentProperties>>(this);
	m_properties->setWindowTitle("Properties");
	m_properties->setObjectName("ComponentProperties");
	addDockWidget(Qt::RightDockWidgetArea, m_properties.get());

	QStatusBar *statusBar = new QStatusBar{this};
	statusBar->addPermanentWidget(m_statusLabel.get(), 1);
	setStatusBar(statusBar);


	// ------------------------------------------------------------------------
	// file menu
	// menu actions
	QIcon iconNew = QIcon::fromTheme("document-new");
	QAction *actionNew = new QAction{iconNew, "New", this};
	connect(actionNew, &QAction::triggered, this, &QmWnd::FileNew);

	QIcon iconLoad = QIcon::fromTheme("document-open");
	QAction *actionLoad = new QAction{iconLoad, "Load...", this};
	connect(actionLoad, &QAction::triggered, this, &QmWnd::FileLoad);

	m_recent.SetRecentMenu(std::make_shared<QMenu>("Load Recent Files", this));

	QIcon iconSave = QIcon::fromTheme("document-save");
	QAction *actionSave = new QAction{iconSave, "Save", this};
	connect(actionSave, &QAction::triggered, this, &QmWnd::FileSave);

	QIcon iconSaveAs = QIcon::fromTheme("document-save-as");
	QAction *actionSaveAs = new QAction{iconSaveAs, "Save as...", this};
	connect(actionSaveAs, &QAction::triggered, this, &QmWnd::FileSaveAs);

	QAction *actionExportSvg = new QAction{"Export Graphics...", this};
	connect(actionExportSvg, &QAction::triggered, [this]()
	{
		auto filedlg = std::make_shared<QFileDialog>(
			this, "Export SVG image", m_recent.GetRecentDir(),
			"SVG Files (*.svg)");
		filedlg->setAcceptMode(QFileDialog::AcceptSave);
		filedlg->setDefaultSuffix("svg");
		filedlg->setFileMode(QFileDialog::AnyFile);

		if(filedlg->exec())
		{
			QStringList files = filedlg->selectedFiles();
			if(files.size() > 0 && files[0] != "")
			{
				QSvgGenerator svggen;
				svggen.setSize(QSize{width(), height()});
				svggen.setFileName(files[0]);

				QPainter paint(&svggen);
				//m_scene->render(&paint);
			}
		}
	});

	QIcon iconExit = QIcon::fromTheme("application-exit");
	QAction *actionExit = new QAction{iconExit, "Exit", this};
	actionExit->setMenuRole(QAction::QuitRole);
	connect(actionExit, &QAction::triggered, [this]()
	{
		this->close();
	});


	QMenu *menuFile = new QMenu{"File", this};
	menuFile->addAction(actionNew);
	menuFile->addSeparator();
	menuFile->addAction(actionLoad);
	menuFile->addMenu(m_recent.GetRecentMenu().get());
	menuFile->addSeparator();
	menuFile->addAction(actionSave);
	menuFile->addAction(actionSaveAs);
	menuFile->addSeparator();
	menuFile->addAction(actionExportSvg);
	menuFile->addSeparator();
	menuFile->addAction(actionExit);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// settings menu
	auto set_gui_theme = [this](const QString& theme) -> void
	{
		if(QStyle* style = QStyleFactory::create(theme); style)
		{
			this->setStyle(style);
			QApplication::setStyle(style);

			m_gui_theme = theme;
		}

		if(theme == GUI_THEME_UNSET)
			m_gui_theme = GUI_THEME_UNSET;
	};

	auto set_gui_native = [this](bool b) -> void
	{
		QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, !b);
		QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, !b);

		m_gui_native = b;
	};

	QMenu *menuSettings = new QMenu{"Settings", this};
	QMenu *menuGuiTheme = new QMenu{"GUI Theme", this};
	QActionGroup *groupTheme = new QActionGroup{this};
	QStringList themes = QStyleFactory::keys();
	themes.push_front(GUI_THEME_UNSET);

	for(const auto& theme : themes)
	{
		QAction *actionTheme = new QAction{theme, this};
		connect(actionTheme, &QAction::triggered, [set_gui_theme, theme]()
		{
			set_gui_theme(theme);
		});

		actionTheme->setCheckable(true);
		if(theme == m_gui_theme)
			actionTheme->setChecked(true);

		menuGuiTheme->addAction(actionTheme);
		groupTheme->addAction(actionTheme);

		if(theme == GUI_THEME_UNSET)
			menuGuiTheme->addSeparator();
	}

	QAction *actionGuiNative = new QAction{"Native GUI", this};
	actionGuiNative->setCheckable(true);
	actionGuiNative->setChecked(m_gui_native);
	connect(actionGuiNative, &QAction::triggered, [set_gui_native](bool checked)
	{
		set_gui_native(checked);
	});

	menuSettings->addMenu(menuGuiTheme);
	menuSettings->addAction(actionGuiNative);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// components menu
	QAction *actionAddCnot = new QAction{"Add CNOT Gate", this};
	connect(actionAddCnot, &QAction::triggered, [this]()
	{
		auto gate = std::make_shared<CNotGate>();
		m_scene->AddGate(gate);
	});

	QAction *actionAddToffoli = new QAction{"Add Toffoli Gate", this};
	connect(actionAddToffoli, &QAction::triggered, [this]()
	{
		auto gate = std::make_shared<ToffoliGate>();
		m_scene->AddGate(gate);
	});

	QMenu *menuComponents = new QMenu{"Components", this};
	menuComponents->addAction(actionAddCnot);
	menuComponents->addAction(actionAddToffoli);
	// ------------------------------------------------------------------------


	// keyboard shortcuts
	actionNew->setShortcut(QKeySequence::New);
	actionLoad->setShortcut(QKeySequence::Open);
	actionSave->setShortcut(QKeySequence::Save);
	actionSaveAs->setShortcut(QKeySequence::SaveAs);
	actionExit->setShortcut(QKeySequence::Quit);


	// apply settings
	set_gui_theme(m_gui_theme);
	set_gui_native(m_gui_native);


	// menu bar
	QMenuBar *menuBar = new QMenuBar{this};
	menuBar->addMenu(menuFile);
	menuBar->addMenu(menuSettings);
	menuBar->addMenu(menuComponents);
	setMenuBar(menuBar);


	// connections
	connect(m_view.get(), &QmView::SignalMouseCoordinates,
		[this](double x, double y) -> void
		{
			SetStatusMessage(QString("x=%1, y=%2.").arg(x, 5).arg(y, 5));
		});

	connect(m_view.get(), &QmView::SignalSelectedItem,
		m_properties->GetWidget(), &ComponentProperties::SelectedItem);


	m_recent.CreateRecentFileMenu(
		[this](const QString& filename)->bool { return this->LoadFile(filename); });
	SetStatusMessage("Ready.");
}


void QmWnd::SetStatusMessage(const QString& msg)
{
	m_statusLabel->setText(msg);
}


void QmWnd::FileNew()
{
	m_recent.SetOpenFile("");
}


void QmWnd::FileLoad()
{
	auto filedlg = std::make_shared<QFileDialog>(
		this, "Load Data", m_recent.GetRecentDir(),
		"XML Files (*.xml);;All Files (* *.*)");
	filedlg->setAcceptMode(QFileDialog::AcceptOpen);
	filedlg->setDefaultSuffix("xml");
	filedlg->setFileMode(QFileDialog::AnyFile);

	if(filedlg->exec())
	{
		QStringList files = filedlg->selectedFiles();
		if(files.size() > 0 && files[0] != "")
		{
			if(LoadFile(files[0]))
			{
				m_recent.SetOpenFile(files[0]);
				m_recent.AddRecentFile(m_recent.GetOpenFile(),
					[this](const QString& filename)->bool { return this->LoadFile(filename); });

				fs::path file{files[0].toStdString()};
				m_recent.SetRecentDir(file.parent_path().string().c_str());
			}
			else
			{
				QMessageBox::critical(this, "Error", "File could not be loaded.");
			}
		}
	}
}


void QmWnd::FileSave()
{
	// no open file, use "save as..." instead
	if(m_recent.GetOpenFile() == "")
	{
		FileSaveAs();
		return;
	}

	if(!SaveFile(m_recent.GetOpenFile()))
		QMessageBox::critical(this, "Error", "File could not be saved.");
}


void QmWnd::FileSaveAs()
{
	auto filedlg = std::make_shared<QFileDialog>(
		this, "Save Data", m_recent.GetRecentDir(),
		"XML Files (*.xml)");
	filedlg->setAcceptMode(QFileDialog::AcceptSave);
	filedlg->setDefaultSuffix("xml");
	filedlg->setFileMode(QFileDialog::AnyFile);

	if(filedlg->exec())
	{
		QStringList files = filedlg->selectedFiles();
		if(files.size() > 0 && files[0] != "")
		{
			if(SaveFile(files[0]))
			{
				m_recent.SetOpenFile(files[0]);
				m_recent.AddRecentFile(m_recent.GetOpenFile(),
					[this](const QString& filename)->bool { return this->LoadFile(filename); });

				fs::path file{files[0].toStdString()};
				m_recent.SetRecentDir(file.parent_path().string().c_str());
			}
			else
			{
				QMessageBox::critical(this, "Error", "File could not be saved.");
			}
		}
	}
}


bool QmWnd::SaveFile(const QString& filename) const
{
	std::ofstream ofstr{filename.toStdString()};
	if(!ofstr)
		return false;

	ptree::ptree prop{};

	// TODO

	ptree::write_xml(ofstr, prop, ptree::xml_writer_make_settings('\t', 1, std::string{"utf-8"}));

	return true;
}


bool QmWnd::LoadFile(const QString& filename)
{
	std::ifstream ifstr{filename.toStdString()};
	if(!ifstr)
		return false;

	ptree::ptree prop{};
	ptree::read_xml(ifstr, prop);

	// TODO

	return true;
}


void QmWnd::closeEvent(QCloseEvent *e)
{
	// ------------------------------------------------------------------------
	// save settings
	QSettings settings{this};

	QByteArray geo{this->saveGeometry()}, state{this->saveState()};
	settings.setValue("wnd_geo", geo);
	settings.setValue("wnd_state", state);
	settings.setValue("wnd_theme", m_gui_theme);
	settings.setValue("wnd_native", m_gui_native);
	settings.setValue("file_recent", m_recent.GetRecentFiles());
	settings.setValue("file_recent_dir", m_recent.GetRecentDir());
	// ------------------------------------------------------------------------

	QMainWindow::closeEvent(e);
}


QmWnd::~QmWnd()
{
}
// ----------------------------------------------------------------------------
