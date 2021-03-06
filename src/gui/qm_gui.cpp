/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include "qm_gui.h"

#include <QtCore/QSettings>
#include <QtCore/QMimeData>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	#include <QtGui/QActionGroup>
#else
	#include <QtWidgets/QActionGroup>
#endif

#include <fstream>
#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "helpers.h"


#define QM_WND_TITLE      "Quantum Algorithms Editor"
#define GUI_THEME_UNSET   "Unset"

// initial position of new component
#define INIT_COMP_POS_X   3
#define INIT_COMP_POS_Y   2


// ----------------------------------------------------------------------------
// main window
// ----------------------------------------------------------------------------
QmWnd::QmWnd(QWidget* pParent)
	: QMainWindow{pParent},
		m_scene{new QmScene{this}},
		m_view{new QmView{m_scene.get(), this}},
		m_statusLabel{std::make_shared<QLabel>(this)}
{
	m_recent.SetOpenFile("");
	SetActiveFile();

	// allow dropping of files onto the main window
	setAcceptDrops(true);

	//m_recent.AddForbiddenDir("/home/tw/Documents");
	RestoreSettings();
}


QmWnd::~QmWnd()
{
	Clear();
}


void QmWnd::LoadPlugins()
{
	m_plugins.LoadPlugins(filesystem::absolute(m_res.GetBinPath()).string());
}


/**
 * create all gui elements
 */
void QmWnd::SetupGUI()
{
	if(auto optIconFile = m_res.FindFile("main.svg"); optIconFile)
		setWindowIcon(QIcon{optIconFile->string().c_str()});

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

	QIcon iconRecent = QIcon::fromTheme("document-open-recent");
	auto menuRecent = std::make_shared<QMenu>("Load Recent Files", this);
	menuRecent->setIcon(iconRecent);
	m_recent.SetRecentMenu(menuRecent);

	QIcon iconSave = QIcon::fromTheme("document-save");
	QAction *actionSave = new QAction{iconSave, "Save", this};
	connect(actionSave, &QAction::triggered, this, &QmWnd::FileSave);

	QIcon iconSaveAs = QIcon::fromTheme("document-save-as");
	QAction *actionSaveAs = new QAction{iconSaveAs, "Save as...", this};
	connect(actionSaveAs, &QAction::triggered, this, &QmWnd::FileSaveAs);

	QIcon iconExportSvg = QIcon::fromTheme("image-x-generic");
	QAction *actionExportSvg = new QAction{iconExportSvg, "Export Graphics...", this};
	connect(actionExportSvg, &QAction::triggered, [this]()
	{
		auto filedlg = std::make_shared<QFileDialog>(
			this, "Export SVG image", GetDocDir(),
			"SVG Files (*.svg)");
		filedlg->setAcceptMode(QFileDialog::AcceptSave);
		filedlg->setDefaultSuffix("svg");
		filedlg->selectFile("untitled");
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
	QAction *actionExit = new QAction{iconExit, "Quit", this};
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

	QIcon iconRename = QIcon::fromTheme("edit-find-replace");
	QAction *actionRename = new QAction(iconRename, "Rename Component...", menuEdit);
	connect(actionRename, &QAction::triggered, this, &QmWnd::ShowRenameDlg);

	menuEdit->addAction(actionCopy);
	menuEdit->addAction(actionPaste);
	menuEdit->addAction(actionDelete);
	menuEdit->addSeparator();
	menuEdit->addAction(actionRename);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// components menu
	// iterate through all component classes and create the matching one
	constexpr const std::size_t num_comps = std::tuple_size<t_all_components>();

	std::array<QAction*, num_comps> compActions{};
	std::array<ComponentType, num_comps> compTypes{};
	std::array<t_uint, num_comps> compMinQBits{};

	[&compActions, &compTypes, &compMinQBits, this]<std::size_t ...idx>
		(const std::index_sequence<idx...>&)
	{
		(
			[&compActions, &compTypes, &compMinQBits, this]()
			{
				using t_comp = std::tuple_element_t<idx, t_all_components>;

				QString menustring = QString("Add %1").arg(t_comp::GetStaticName());
				QString menuicon = QString("%1.svg").arg(t_comp::GetStaticIdent());

				// action
				QAction *actionComp = new QAction{menustring, this};

				// icon
				if(auto optIconFile = m_res.FindFile(menuicon.toStdString()); optIconFile)
					actionComp->setIcon(QIcon{optIconFile->string().c_str()});

				// item clicked -> add a new component
				connect(actionComp, &QAction::triggered, [this]()
				{
					if(QuantumComponentItem *comp = new(std::nothrow) t_comp{}; comp)
					{
						comp->SetGridPos(INIT_COMP_POS_X, INIT_COMP_POS_Y);
						m_view->AddQuantumComponent(comp, true);

						QPointF safePos = m_view->GetSafePos(comp, comp->scenePos());
						comp->setPos(snap_to_grid(safePos));

						WorkspaceChanged(true);
					}
					else
					{
						QMessageBox::critical(this, "Error",
							QString("Component \"%1\" could not be created.")
								.arg(t_comp::GetStaticName()));
					}
				});

				std::get<idx>(compActions) = actionComp;
				std::get<idx>(compTypes) = t_comp::GetStaticType();
				std::get<idx>(compMinQBits) = t_comp::GetMinNumQBits();
			}(),
		...);
	}(std::make_index_sequence<num_comps>());

	QMenu *menuComponents = new QMenu{"Components", this};
	// actions added together with toolbar below
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// plugin components menu
	std::vector<QAction*> plugincompActions{};

	for(const auto& [comp_ident, comp_name] : m_plugins.GetComponentNames())
	{
		QString menustring = QString("Add %1").arg(comp_name.c_str());
		QString menuicon = QString("%1.svg").arg(comp_ident.c_str());

		// action
		QAction *actionComp = new QAction{menustring, this};

		// icon
		if(auto optIconFile = m_res.FindFile(menuicon.toStdString()); optIconFile)
			actionComp->setIcon(QIcon{optIconFile->string().c_str()});

		// item clicked -> add a new component
		std::string comp_ident_local = comp_ident;
		connect(actionComp, &QAction::triggered, [this, comp_ident_local]()
		{
			QuantumComponentItem *comp = m_plugins.CreateComponent(comp_ident_local);
			comp->SetGridPos(INIT_COMP_POS_X, INIT_COMP_POS_Y);
			m_view->AddQuantumComponent(comp, true);

			QPointF safePos = m_view->GetSafePos(comp, comp->scenePos());
			comp->setPos(snap_to_grid(safePos));

			WorkspaceChanged(true);
		});

		plugincompActions.emplace_back(actionComp);
	}

	QMenu *menuPluginComponents = nullptr;
	if(plugincompActions.size())
	{
		menuPluginComponents = new QMenu("Plugin Components", this);

		for(QAction *action : plugincompActions)
			menuPluginComponents->addAction(action);
	}

	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// calculate menu
	QIcon iconCalc = QIcon::fromTheme("media-playback-start");
	QAction *actionCalculateSelected = new QAction{iconCalc, "Calculate Selected Circuit", this};
	connect(actionCalculateSelected, &QAction::triggered, m_view.get(), &QmView::CalculateCurItem);

	QIcon iconCalcAll = QIcon::fromTheme("media-seek-forward");
	QAction *actionCalculateAll = new QAction{iconCalcAll, "Calculate All Circuits", this};
	connect(actionCalculateAll, &QAction::triggered, this, &QmWnd::CalculateAllCircuits);

	QIcon iconAutoCalc = QIcon::fromTheme("accessories-calculator");
	QAction *actionAutoCalculate = new QAction{iconAutoCalc, "Automatically Calculate Circuits", this};
	actionAutoCalculate->setCheckable(true);
	actionAutoCalculate->setChecked(m_auto_calc);
	connect(actionAutoCalculate, &QAction::toggled, [this](bool checked)
	{
		m_auto_calc = checked;
	});

	QMenu *menuCalculate = new QMenu{"Calculate", this};
	menuCalculate->addAction(actionCalculateSelected);
	menuCalculate->addAction(actionCalculateAll);
	menuCalculate->addSeparator();
	menuCalculate->addAction(actionAutoCalculate);
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

	ComponentType lastType{};
	t_uint lastQBits{1};
	for(std::size_t i=0; i<compActions.size(); ++i)
	{
		QAction* compAction = compActions[i];
		ComponentType compType = compTypes[i];
		t_uint compQBits = compMinQBits[i];

		if(i > 0 && (lastType != compType || lastQBits != compQBits))
		{
			menuComponents->addSeparator();
			toolbarComponents->addSeparator();
		}

		menuComponents->addAction(compAction);
		toolbarComponents->addAction(compAction);
		lastType = compType;
		lastQBits = compQBits;
	}

	QToolBar *toolbarCalc = new QToolBar{"Calculate", this};
	toolbarCalc->setObjectName("CalculateToolbar");
	toolbarCalc->addAction(actionCalculateSelected);
	toolbarCalc->addAction(actionCalculateAll);

	addToolBar(toolbarFile);
	addToolBar(toolbarComponents);
	addToolBar(toolbarCalc);
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

	QIcon iconSettings = QIcon::fromTheme("preferences-system");
	QAction *actionSettings = new QAction{iconSettings, "Settings...", this};
	actionSettings->setMenuRole(QAction::PreferencesRole);
	connect(actionSettings, &QAction::triggered, this, &QmWnd::ShowSettings);

	QIcon iconGuiTheme = QIcon::fromTheme("preferences-desktop-theme");
	QMenu *menuGuiTheme = new QMenu{"GUI Theme", this};
	menuGuiTheme->setIcon(iconGuiTheme);
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

	// set native gui
	QIcon iconGuiNative = QIcon::fromTheme("preferences-desktop");
	QAction *actionGuiNative = new QAction{iconGuiNative, "Native GUI", this};
	actionGuiNative->setCheckable(true);
	actionGuiNative->setChecked(m_gui_native);
	connect(actionGuiNative, &QAction::triggered, [set_gui_native](bool checked)
	{
		set_gui_native(checked);
	});

	// clear settings
	QIcon iconClearSettings = QIcon::fromTheme("edit-clear");
	QAction *actionClearSettings = new QAction{iconClearSettings,
		"Clear All Settings", this};
	connect(actionClearSettings, &QAction::triggered, [this]()
	{
		QSettings{this}.clear();
	});

	// restore layout
	QIcon iconRestoreLayout = QIcon::fromTheme("view-restore");
	QAction *actionRestoreLayout = new QAction{iconRestoreLayout,
		"Restore GUI Layout", this};
	connect(actionRestoreLayout, &QAction::triggered, [this]()
	{
		if(m_default_window_state.size())
			this->restoreState(m_default_window_state);
	});


	// tool menu
	QIcon iconTools = QIcon::fromTheme("applications-system");
	QMenu *menuTools = new QMenu{"Tools", this};
	menuTools->setIcon(iconTools);
	menuTools->addAction(toolbarFile->toggleViewAction());
	menuTools->addAction(toolbarComponents->toggleViewAction());
	menuTools->addAction(toolbarCalc->toggleViewAction());
	menuTools->addSeparator();
	menuTools->addAction(m_properties->toggleViewAction());

	menuSettings->addAction(actionSettings);
	menuSettings->addAction(actionClearSettings);
	menuSettings->addSeparator();
	menuSettings->addMenu(menuTools);
	menuSettings->addSeparator();
	menuSettings->addMenu(menuGuiTheme);
	menuSettings->addAction(actionGuiNative);
	menuSettings->addAction(actionRestoreLayout);
	// ------------------------------------------------------------------------


	// ------------------------------------------------------------------------
	// about menu
	QMenu *menuHelp = new QMenu{"Help", this};

	QIcon iconAbout = QIcon::fromTheme("help-about");
	QAction *actionAbout = new QAction{iconAbout, "About...", this};
	actionAbout->setMenuRole(QAction::AboutRole);
	connect(actionAbout, &QAction::triggered, this, &QmWnd::ShowAbout);

	menuHelp->addAction(actionAbout);
	// ------------------------------------------------------------------------


	// keyboard shortcuts
	actionNew->setShortcut(QKeySequence::New);
	actionLoad->setShortcut(QKeySequence::Open);
	actionSave->setShortcut(QKeySequence::Save);
	actionSaveAs->setShortcut(QKeySequence::SaveAs);
	actionExit->setShortcut(QKeySequence::Quit);
	actionSettings->setShortcut(QKeySequence::Preferences);

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
	if(menuPluginComponents)
		menuBar->addMenu(menuPluginComponents);
	menuBar->addMenu(menuCalculate);
	menuBar->addMenu(menuSettings);
	menuBar->addMenu(menuHelp);
	setMenuBar(menuBar);


	// connections
	connect(m_view.get(), &QmView::SignalMouseCoordinates,
		[this](qreal scene_x, qreal scene_y) -> void
		{
			auto [tile_x, tile_y] =  get_grid_indices(
				scene_x, scene_y, g_raster_size, g_raster_size);

			SetStatusMessage(QString(
				"Tile: (%1, %2), scene: (%3, %4).")
				.arg(tile_x, 2).arg(tile_y, 2)
				.arg(scene_x, 5).arg(scene_y, 5));
		});

	// signals to read and write component properties
	connect(m_view.get(), &QmView::SignalSelectedItem,
		m_properties->GetWidget(), &ComponentProperties::SelectedItem);
	connect(m_view.get(), &QmView::SignalNewResults,
		m_properties->GetWidget(), &ComponentProperties::UpdateResults);
	connect(m_view.get(), &QmView::SignalWorkspaceChanged,
		this, &QmWnd::WorkspaceChanged);
	connect(m_properties->GetWidget(), &ComponentProperties::SignalConfigChanged,
		m_view.get(), &QmView::SetCurItemConfig);


	// restore recent files menu
	m_recent.CreateRecentFileMenu(
		[this](const QString& filename) -> bool
	{
		return this->FileLoadRecent(filename);
	});

	// save initial default window state
	m_default_window_state = saveState();

	// restore saved window state and geometry
	if(m_saved_window_geometry.size())
		restoreGeometry(m_saved_window_geometry);
	if(m_saved_window_state.size())
		restoreState(m_saved_window_state);

	SetStatusMessage("Ready.");
}


/**
 * restore saved settings
 */
void QmWnd::RestoreSettings()
{
	QSettings settings{this};

	if(settings.contains("wnd_geo"))
	{
		m_saved_window_geometry = 
			settings.value("wnd_geo").toByteArray();
	}
	else
	{
		resize(1024, 768);
	}

	if(settings.contains("wnd_state"))
	{
		m_saved_window_state =
			settings.value("wnd_state").toByteArray();
	}

	if(settings.contains("wnd_theme"))
		m_gui_theme = settings.value("wnd_theme").toString();

	if(settings.contains("wnd_native"))
		m_gui_native = settings.value("wnd_native").toBool();

	if(settings.contains("calc_auto"))
		m_auto_calc = settings.value("calc_auto").toBool();

	if(settings.contains("file_recent"))
		m_recent.SetRecentFiles(settings.value("file_recent").toStringList());

	if(settings.contains("file_recent_dir"))
		m_recent.SetRecentDir(settings.value("file_recent_dir").toString());


	// restore settings from settings dialog
	ShowSettings(true);
}


void QmWnd::SaveSettings()
{
	QSettings settings{this};

	QByteArray geo{this->saveGeometry()},
		state{this->saveState()};

	settings.setValue("wnd_geo", geo);
	settings.setValue("wnd_state", state);
	settings.setValue("wnd_theme", m_gui_theme);
	settings.setValue("wnd_native", m_gui_native);
	settings.setValue("calc_auto", m_auto_calc);
	settings.setValue("file_recent", m_recent.GetRecentFiles());
	settings.setValue("file_recent_dir", m_recent.GetRecentDir());
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
	WorkspaceChanged(false);
}


void QmWnd::FileNew()
{
	if(!AskUnsaved())
		return;

	Clear();

	if(QuantumComponentItem *state = new(std::nothrow) InputStates(); state)
	{
		state->SetGridPos(INIT_COMP_POS_X, INIT_COMP_POS_Y);
		m_view->AddQuantumComponent(state);
	}

	WorkspaceChanged(false);
}


bool QmWnd::FileLoad()
{
	if(!AskUnsaved())
		return false;

	auto filedlg = std::make_shared<QFileDialog>(
		this, "Load Data", GetDocDir(),
		"XML Files (*.xml);;All Files (* *.*)");
	filedlg->setAcceptMode(QFileDialog::AcceptOpen);
	filedlg->setDefaultSuffix("xml");
	filedlg->setFileMode(QFileDialog::AnyFile);

	if(!filedlg->exec())
		return false;

	QStringList files = filedlg->selectedFiles();
	if(files.size() == 0 || files[0] == "")
		return false;

	Clear();
	if(LoadFile(files[0]))
	{
		fs::path file{files[0].toStdString()};
		m_recent.SetRecentDir(file.parent_path().string().c_str());
		m_recent.SetOpenFile(files[0]);

		m_recent.AddRecentFile(m_recent.GetOpenFile(),
			[this](const QString& filename) -> bool
		{
			return this->FileLoadRecent(filename);
		});

		WorkspaceChanged(false);
		return true;
	}
	else
	{
		QMessageBox::critical(this, "Error",
			QString("File \"%1\" could not be loaded.").arg(files[0]));
		return false;
	}
}


/**
 * an item from the recent files menu has been clicked
 */
bool QmWnd::FileLoadRecent(const QString& filename)
{
	if(!AskUnsaved())
		return false;

	this->Clear();

	if(LoadFile(filename))
	{
		m_recent.SetOpenFile(filename);
		WorkspaceChanged(false);

		return true;
	}
	else
	{
		QMessageBox::critical(this, "Error",
			QString("File \"%1\" could not be loaded.").arg(filename));
		return false;
	}
}


bool QmWnd::FileSave()
{
	// no open file, use "save as..." instead
	if(m_recent.GetOpenFile() == "")
		return FileSaveAs();

	if(SaveFile(m_recent.GetOpenFile()))
	{
		WorkspaceChanged(false);
		return true;
	}
	else
	{
		QMessageBox::critical(this, "Error", "File could not be saved.");
		return false;
	}
}


bool QmWnd::FileSaveAs()
{
	auto filedlg = std::make_shared<QFileDialog>(
		this, "Save Data", GetDocDir(),
		"XML Files (*.xml)");
	filedlg->setAcceptMode(QFileDialog::AcceptSave);
	filedlg->setDefaultSuffix("xml");
	filedlg->selectFile("untitled");
	filedlg->setFileMode(QFileDialog::AnyFile);

	if(!filedlg->exec())
		return false;

	QStringList files = filedlg->selectedFiles();
	if(files.size() == 0 || files[0] == "")
		return false;

	if(SaveFile(files[0]))
	{
		fs::path file{files[0].toStdString()};
		m_recent.SetRecentDir(file.parent_path().string().c_str());
		m_recent.SetOpenFile(files[0]);

		m_recent.AddRecentFile(m_recent.GetOpenFile(),
			[this](const QString& filename) -> bool
		{
			return this->FileLoadRecent(filename);
		});

		WorkspaceChanged(false);
		return true;
	}
	else
	{
		QMessageBox::critical(this, "Error",
			QString("File \"%1\" could not be saved.").arg(files[0]));
		return false;
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

		auto [pos_x, pos_y] = gate->GetGridPos();
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
			else if(std::holds_alternative<t_cplx>(config.value))
			{
				std::string key = "component." + config.key;
				propGate.put<t_cplx>(
					key, std::get<t_cplx>(config.value));
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

			// component identifier
			std::string id = propGate.get<std::string>("<xmlattr>.ident", "");

			// try to create a built-in component
			QuantumComponentItem *gate = QuantumComponentItem::create(id);

			// try to create a plug-in component
			if(!gate)
				gate = m_plugins.CreateComponent(id);

			if(!gate)
			{
				QMessageBox::critical(this, "Error",
					QString("Component \"%1\" could not be created.").arg(id.c_str()));

				continue;
			}

			// get configuration settings
			ComponentConfigs configs = gate->GetConfig();

			for(ComponentConfig& config : configs.configs)
			{
				// test for all possible types of the variant
				// and set the new value with the same type
				if(std::holds_alternative<t_real>(config.value))
					config.value = propGate.get<t_real>(config.key, 0);
				else if(std::holds_alternative<t_cplx>(config.value))
					config.value = propGate.get<t_cplx>(config.key, 0);
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
			gate->SetGridPos(pos_x, pos_y);

			m_view->AddQuantumComponent(gate, true);
		}

		WorkspaceChanged(true);
	}

	m_view->FitAreaToScene();
	return true;
}


/**
 * show settings dialog
 */
void QmWnd::ShowSettings(bool only_create)
{
	if(!m_settings)
	{
		m_settings = std::make_shared<Settings>(this);
		connect(m_settings.get(), &Settings::SignalApplySettings,
			this, &QmWnd::ApplySettings);

		m_settings->AddCheckbox("settings/reverse_state_numbering",
			"Number qubits from LSB (right) to MSB (left).", g_reverse_state_numbering);
		m_settings->AddCheckbox("settings/snap_on_move",
			"Snap to grid while dragging.", g_snap_on_move);
		m_settings->AddCheckbox("settings/keep_gates_on_states",
			"Keep gates on input state component.", g_keep_gates_on_states);
		m_settings->AddSpinbox("settings/raster_size",
			"Components raster size:", (int)g_raster_size, 1, 1000, 1);
		m_settings->AddSpinbox("settings/precision_gui",
			"Number precision:", g_prec_gui, 0, 99, 1);
		m_settings->AddDoubleSpinbox("settings/classical_threshold",
			"Classical threshold probability:", g_classical_threshold, 0, 1., 1e-2);
		//m_settings->AddDoubleSpinbox("settings/epsilon",
		//	"Calculation epsilon:", g_eps, g_eps, 1., 1e-6);
		m_settings->FinishSetup();
	}

	if(!only_create)
		show_dialog(m_settings.get());
}


/**
 * apply changed settings from the settings dialog
 */
void QmWnd::ApplySettings()
{
	if(!m_settings)
		return;

	g_reverse_state_numbering = m_settings->GetValue("settings/reverse_state_numbering").
		value<decltype(g_snap_on_move)>();
	g_snap_on_move = m_settings->GetValue("settings/snap_on_move").
		value<decltype(g_snap_on_move)>();
	g_keep_gates_on_states = m_settings->GetValue("settings/keep_gates_on_states").
		value<decltype(g_keep_gates_on_states)>();
	g_raster_size = m_settings->GetValue("settings/raster_size").value<int>();
	g_prec_gui = m_settings->GetValue("settings/precision_gui").
		value<decltype(g_prec_gui)>();
	//g_eps = m_settings->GetValue("settings/epsilon").
	//	value<decltype(g_eps)>();

	update();
}


/**
 * calculate all circuits in the scene
 */
void QmWnd::CalculateAllCircuits()
{
	for(auto* comp : m_scene->GetAllInputStates())
		m_view->Calculate(comp);
}


/**
 * indicate that the open workspace has unsaved changes
 */
void QmWnd::WorkspaceChanged(bool changed)
{
	if(m_auto_calc)
		CalculateAllCircuits();

	SetWindowModified(changed);
	SetActiveFile();
}


/**
 * show about dialog
 */
void QmWnd::ShowAbout()
{
	if(!m_about)
	{
		QIcon icon = windowIcon();
		m_about = std::make_shared<About>(this, &icon);
	}

	show_dialog(m_about.get());
}


/**
 * show about dialog
 */
void QmWnd::ShowRenameDlg()
{
	if(!m_rename)
	{
		m_rename = std::make_shared<Rename>(this);

		connect(m_view.get(), &QmView::SignalSelectedItem,
			m_rename.get(), &Rename::SelectedItem);

		auto *selected_comp = m_view->GetCurItem();
		auto *input_comp = m_scene->GetCorrespondingInputState(selected_comp);
		m_rename->SelectedItem(selected_comp, input_comp);
	}

	show_dialog(m_rename.get());
}


/**
 * show the active file in the window title
 */
void QmWnd::SetActiveFile()
{
	const QString& filename = m_recent.GetOpenFile();
	setWindowFilePath(filename);

	QString mod{IsWindowModified() ? " *" : ""};
	if(filename == "")
	{
		setWindowTitle(QString(QM_WND_TITLE "%1").arg(mod));
	}
	else
	{
		setWindowTitle(QString(QM_WND_TITLE " \u2014 %1%2")
			.arg(QFileInfo{filename}.fileName())
			.arg(mod));
	}
}


/**
 * ask to save unsaved changes
 * @return ok to continue?
 */
bool QmWnd::AskUnsaved()
{
	// unsaved changes?
	if(IsWindowModified())
	{
		QMessageBox::StandardButton btn = QMessageBox::question(
			this, "Save Changes?",
			"The workspace has unsaved changes. Save them now?",
			QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
			QMessageBox::Yes);

		if(btn == QMessageBox::Yes)
		{
			if(!FileSave())
				return false;
		}
		else if(btn == QMessageBox::Cancel)
			return false;
	}

	return true;
}


/**
 * get the directory to save documents in
 */
QString QmWnd::GetDocDir()
{
	if(g_use_recent_dir)
		return m_recent.GetRecentDir();

	// use either the documents or the home dir
	QString path;
	QStringList dirs = QStandardPaths::standardLocations(
		QStandardPaths::DocumentsLocation);
	if(dirs.size())
		path = dirs[0];
	else
		path = QDir::homePath();

	QString subdir = "qm_files";
	QDir dir(path);
	if(!dir.exists(subdir))
	{
		QMessageBox::StandardButton btn = 
			QMessageBox::question(this, "Create document directory",
				QString("Create the directory \"%1\" under \"%2\"?").
					arg(subdir).
					arg(dir.absolutePath()),
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
				QMessageBox::Yes);

		if(btn == QMessageBox::Yes)
		{
			dir.mkdir(subdir);
			dir.cd(subdir);
		}
	}
	else
	{
		dir.cd(subdir);
	}

	return dir.absolutePath();
}


void QmWnd::closeEvent(QCloseEvent *evt)
{
	if(!AskUnsaved())
	{
		evt->ignore();
		return;
	}

	SaveSettings();
	QMainWindow::closeEvent(evt);
}


/**
 * accept a file dropped onto the main window
 * @see https://doc.qt.io/qt-5/dnd.html
 */
void QmWnd::dragEnterEvent(QDragEnterEvent *evt)
{
	if(!evt)
		return;

	// accept urls
	if(evt->mimeData()->hasUrls())
		evt->accept();

	QMainWindow::dragEnterEvent(evt);
}


/**
 * accept a file dropped onto the main window
 * @see https://doc.qt.io/qt-5/dnd.html
 */
void QmWnd::dropEvent(QDropEvent *evt)
{
	if(!evt)
		return;

	// get mime data dropped on the main window
	if(const QMimeData* dat = evt->mimeData(); dat && dat->hasUrls())
	{
		// get the list of urls dropped on the main window
		if(QList<QUrl> urls = dat->urls(); urls.size() > 0)
		{
			// use the first url for the file name
			const QUrl& url = *urls.begin();
			QString filename = url.path();

			// load the dropped file
			Clear();
			if(LoadFile(filename))
			{
				fs::path file{filename.toStdString()};
				m_recent.SetRecentDir(file.parent_path().string().c_str());
				m_recent.SetOpenFile(filename);

				m_recent.AddRecentFile(m_recent.GetOpenFile(),
				[this](const QString& filename) -> bool
				{
					return this->FileLoadRecent(filename);
				});

				WorkspaceChanged(false);
			}
			else
			{
				QMessageBox::critical(this, "Error",
					QString("File \"%1\" could not be loaded.").arg(filename));
			}
		}
	}

	QMainWindow::dropEvent(evt);
}

// ----------------------------------------------------------------------------
