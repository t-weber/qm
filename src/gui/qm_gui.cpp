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
#include <QtWidgets/QToolBar>
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

#include "helpers.h"


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
}


/**
 * create all gui elements
 */
void QmWnd::SetupGUI()
{
	setWindowTitle("QM");
	setCentralWidget(m_view.get());

	m_properties = std::make_shared<DockWidgetWrapper<ComponentProperties>>(this);
	m_properties->setWindowTitle("Properties");
	m_properties->setObjectName("ComponentProperties");
	addDockWidget(Qt::RightDockWidgetArea, m_properties.get());

	QStatusBar *statusBar = new QStatusBar{this};
	statusBar->addPermanentWidget(m_statusLabel.get(), 1);
	statusBar->setSizeGripEnabled(true);
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
				m_scene->render(&paint);
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
	// edit menu
	QMenu *menuEdit = new QMenu{"Edit", this};

	QIcon iconCopy = QIcon::fromTheme("edit-copy");
	QAction *actionCopy = new QAction(iconCopy, "Copy Component", menuEdit);
	connect(actionCopy, &QAction::triggered, m_view.get(), &QmView::CopyCurItem);

	QIcon iconPaste = QIcon::fromTheme("edit-paste");
	QAction *actionPaste = new QAction(iconPaste, "Paste Component", menuEdit);
	connect(actionPaste, &QAction::triggered, m_view.get(), &QmView::PasteItem);

	QIcon iconDelete = QIcon::fromTheme("edit-delete");
	QAction *actionDelete = new QAction(iconDelete, "Delete Component", menuEdit);
	connect(actionDelete, &QAction::triggered, m_view.get(), &QmView::DeleteCurItem);

	menuEdit->addAction(actionCopy);
	menuEdit->addAction(actionPaste);
	menuEdit->addSeparator();
	menuEdit->addAction(actionDelete);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// components menu
	QAction *actionAddInputStates = new QAction{"Add Input Qubits", this};
	if(auto optIconFile = m_res.FindFile("input_state.svg"); optIconFile)
		actionAddInputStates->setIcon(QIcon{optIconFile->string().c_str()});
	connect(actionAddInputStates, &QAction::triggered, [this]()
	{
		QuantumComponentItem *state = new InputStates();
		m_scene->AddQuantumComponent(state);
	});

	QAction *actionAddHadamard = new QAction{"Add Hadamard Gate", this};
	if(auto optIconFile = m_res.FindFile("hadamard.svg"); optIconFile)
		actionAddHadamard->setIcon(QIcon{optIconFile->string().c_str()});
	connect(actionAddHadamard, &QAction::triggered, [this]()
	{
		QuantumComponentItem *gate = new HadamardGate();
		m_scene->AddQuantumComponent(gate);
	});

	QAction *actionAddPauli = new QAction{"Add Pauli Gate", this};
	if(auto optIconFile = m_res.FindFile("pauli_x.svg"); optIconFile)
		actionAddPauli->setIcon(QIcon{optIconFile->string().c_str()});
	connect(actionAddPauli, &QAction::triggered, [this]()
	{
		QuantumComponentItem *gate = new PauliGate();
		m_scene->AddQuantumComponent(gate);
	});

	QAction *actionAddCnot = new QAction{"Add CNOT Gate", this};
	if(auto optIconFile = m_res.FindFile("cnot.svg"); optIconFile)
		actionAddCnot->setIcon(QIcon{optIconFile->string().c_str()});
	connect(actionAddCnot, &QAction::triggered, [this]()
	{
		QuantumComponentItem *gate = new CNotGate();
		m_scene->AddQuantumComponent(gate);
	});

	QAction *actionAddToffoli = new QAction{"Add Toffoli Gate", this};
	if(auto optIconFile = m_res.FindFile("toffoli.svg"); optIconFile)
		actionAddToffoli->setIcon(QIcon{optIconFile->string().c_str()});
	connect(actionAddToffoli, &QAction::triggered, [this]()
	{
		QuantumComponentItem *gate = new ToffoliGate();
		m_scene->AddQuantumComponent(gate);
	});

	QMenu *menuComponents = new QMenu{"Components", this};
	menuComponents->addAction(actionAddInputStates);
	menuComponents->addSeparator();
	menuComponents->addAction(actionAddHadamard);
	menuComponents->addAction(actionAddPauli);
	menuComponents->addAction(actionAddCnot);
	menuComponents->addAction(actionAddToffoli);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// tool bar
	QToolBar *toolbarFile = new QToolBar{"File", this};
	toolbarFile->setObjectName("FileToolbar");
	toolbarFile->addAction(actionNew);
	toolbarFile->addAction(actionLoad);
	toolbarFile->addAction(actionSave);
	toolbarFile->addAction(actionSaveAs);

	QToolBar *toolbarComponents = new QToolBar{"Components", this};
	toolbarComponents->setObjectName("ComponentsToolbar");
	toolbarComponents->addAction(actionAddHadamard);
	toolbarComponents->addAction(actionAddPauli);
	toolbarComponents->addAction(actionAddCnot);
	toolbarComponents->addAction(actionAddToffoli);

	addToolBar(toolbarFile);
	addToolBar(toolbarComponents);
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

	menuSettings->addAction(toolbarFile->toggleViewAction());
	menuSettings->addAction(toolbarComponents->toggleViewAction());
	menuSettings->addAction(m_properties->toggleViewAction());
	menuSettings->addSeparator();
	menuSettings->addMenu(menuGuiTheme);
	menuSettings->addAction(actionGuiNative);
	// ------------------------------------------------------------------------


	// keyboard shortcuts
	actionNew->setShortcut(QKeySequence::New);
	actionLoad->setShortcut(QKeySequence::Open);
	actionSave->setShortcut(QKeySequence::Save);
	actionSaveAs->setShortcut(QKeySequence::SaveAs);
	actionExit->setShortcut(QKeySequence::Quit);

	actionCopy->setShortcut(QKeySequence::Copy);
	actionPaste->setShortcut(QKeySequence::Paste);
	actionDelete->setShortcut(QKeySequence::Delete);


	// apply settings
	set_gui_theme(m_gui_theme);
	set_gui_native(m_gui_native);


	// menu bar
	QMenuBar *menuBar = new QMenuBar{this};
	menuBar->addMenu(menuFile);
	menuBar->addMenu(menuEdit);
	menuBar->addMenu(menuComponents);
	menuBar->addMenu(menuSettings);
	setMenuBar(menuBar);


	// connections
	connect(m_view.get(), &QmView::SignalMouseCoordinates,
		[this](qreal scene_x, qreal scene_y) -> void
		{
			auto [tile_x, tile_y] =  get_grid_indices(scene_x, scene_y);

			SetStatusMessage(QString(
				"Tile: (%1, %2), scene: (%3, %4).")
				.arg(tile_x, 2).arg(tile_y, 2)
				.arg(scene_x, 5).arg(scene_y, 5));
		});

	// signals to read and write component properties
	connect(m_view.get(), &QmView::SignalSelectedItem,
		m_properties->GetWidget(), &ComponentProperties::SelectedItem);
	connect(m_properties->GetWidget(), &ComponentProperties::SignalConfigChanged,
		m_view.get(), &QmView::SetCurItemConfig);


	m_recent.CreateRecentFileMenu(
		[this](const QString& filename) -> bool
	{
		this->Clear();
		return this->LoadFile(filename);
	});
	SetStatusMessage("Ready.");
}


void QmWnd::SetStatusMessage(const QString& msg)
{
	m_statusLabel->setText(msg);
}


void QmWnd::Clear()
{
	m_view->Clear();
	m_scene->Clear();

	m_recent.SetOpenFile("");
}


void QmWnd::FileNew()
{
	Clear();

	QuantumComponentItem *state = new InputStates();
	m_scene->AddQuantumComponent(state);
}


void QmWnd::FileLoad()
{
	auto filedlg = std::make_shared<QFileDialog>(
		this, "Load Data", m_recent.GetRecentDir(),
		"XML Files (*.xml);;All Files (* *.*)");
	filedlg->setAcceptMode(QFileDialog::AcceptOpen);
	filedlg->setDefaultSuffix("xml");
	filedlg->setFileMode(QFileDialog::AnyFile);

	if(!filedlg->exec())
		return;

	QStringList files = filedlg->selectedFiles();
	if(files.size() == 0 || files[0] == "")
		return;

	Clear();
	if(LoadFile(files[0]))
	{
		m_recent.SetOpenFile(files[0]);
		m_recent.AddRecentFile(m_recent.GetOpenFile(),
			[this](const QString& filename) -> bool
		{
			this->Clear();
			return this->LoadFile(filename);
		});

		fs::path file{files[0].toStdString()};
		m_recent.SetRecentDir(file.parent_path().string().c_str());
	}
	else
	{
		QMessageBox::critical(this, "Error",
			QString("File \"%1\" could not be loaded.").arg(files[0]));
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

	if(!filedlg->exec())
		return;

	QStringList files = filedlg->selectedFiles();
	if(files.size() == 0 || files[0] == "")
		return;

	if(SaveFile(files[0]))
	{
		m_recent.SetOpenFile(files[0]);
		m_recent.AddRecentFile(m_recent.GetOpenFile(),
			[this](const QString& filename) -> bool
		{
			this->Clear();
			return this->LoadFile(filename);
		});

		fs::path file{files[0].toStdString()};
		m_recent.SetRecentDir(file.parent_path().string().c_str());
	}
	else
	{
		QMessageBox::critical(this, "Error",
			QString("File \"%1\" could not be saved.").arg(files[0]));
	}
}


bool QmWnd::SaveFile(const QString& filename) const
{
	namespace ptree = boost::property_tree;

	std::ofstream ofstr{filename.toStdString()};
	if(!ofstr)
		return false;

	ptree::ptree prop{};

	// save gates
	ptree::ptree propGates{};
	const std::vector<QuantumComponentItem*>& gates =
		m_scene->GetQuantumComponents();
	for(const QuantumComponentItem *gate : gates)
	{
		std::string component_type = "unknown";
		switch(gate->GetType())
		{
			case ComponentType::STATE:
				component_type = "state";
				break;
			case ComponentType::GATE:
				component_type = "gate";
				break;
		}

		ptree::ptree propGate{};
		propGate.put<std::string>(
			"component.<xmlattr>.type", component_type);
		propGate.put<std::string>(
			"component.<xmlattr>.ident", gate->GetIdent());

		auto [pos_x, pos_y] =  get_grid_indices(gate->scenePos());
		propGate.put<t_int>("component.pos_x", pos_x);
		propGate.put<t_int>("component.pos_y", pos_y);

		// get configuration settings
		const ComponentConfigs& configs = gate->GetConfig();
		for(const ComponentConfig& config : configs.configs)
		{
			// test for all possible types of the variant
			if(std::holds_alternative<t_real>(config.value))
			{
				std::string key = "component." + config.key;
				propGate.put<t_real>(
					key, std::get<t_real>(config.value));
			}
			else if(std::holds_alternative<t_int>(config.value))
			{
				std::string key = "component." + config.key;
				propGate.put<t_int>(
					key, std::get<t_int>(config.value));
			}
			else if(std::holds_alternative<t_uint>(config.value))
			{
				std::string key = "component." + config.key;
				propGate.put<t_uint>(
					key, std::get<t_uint>(config.value));
			}
			else if(std::holds_alternative<std::string>(config.value))
			{
				std::string key = "component." + config.key;
				propGate.put<std::string>(
					key, std::get<std::string>(config.value));
			}
		}

		propGates.push_back(*propGate.begin());
	}
	prop.put_child("qm.components", propGates);

	ptree::write_xml(ofstr, prop,
		ptree::xml_writer_make_settings(
			'\t', 1, std::string{"utf-8"}));

	return true;
}


bool QmWnd::LoadFile(const QString& filename)
{
	namespace ptree = boost::property_tree;

	if(!m_scene || !m_view)
		return false;

	std::ifstream ifstr{filename.toStdString()};
	if(!ifstr)
		return false;

	ptree::ptree prop{};
	ptree::read_xml(ifstr, prop);

	// load gates
	if(auto propGates = prop.get_child_optional("qm.components"); propGates)
	{
		for(const auto& [tagGate, propGate] : *propGates)
		{
			// ignore unknown tags
			if(tagGate != "component")
				continue;

			std::string id = propGate.get<std::string>("<xmlattr>.ident", "");
			QuantumComponentItem *gate = QuantumComponentItem::create(id);

			if(gate)
			{
				// get configuration settings
				ComponentConfigs configs = gate->GetConfig();
				for(ComponentConfig& config : configs.configs)
				{
					// test for all possible types of the variant
					// and set the new value with the same type
					if(std::holds_alternative<t_real>(config.value))
						config.value = propGate.get<t_real>(config.key, 0);
					else if(std::holds_alternative<t_int>(config.value))
						config.value = propGate.get<t_int>(config.key, 0);
					else if(std::holds_alternative<t_uint>(config.value))
						config.value = propGate.get<t_uint>(config.key, 0);
					else if(std::holds_alternative<std::string>(config.value))
						config.value = propGate.get<std::string>(config.key, "");
				}
				gate->SetConfig(configs);

				t_int pos_x = propGate.get<t_int>("pos_x", 0);
				t_int pos_y = propGate.get<t_int>("pos_y", 0);

				QPointF posScene(pos_x*g_raster_size, pos_y*g_raster_size);
				gate->setPos(posScene);

				m_scene->AddQuantumComponent(gate);
			}
		}
	}

	m_view->FitAreaToScene();
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
