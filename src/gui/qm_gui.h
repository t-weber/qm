/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_GUI_H__
#define __QM_GUI_H__

#include <QtCore/QByteArray>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>

#include <memory>
#include <vector>

#include "components.h"
#include "components_plugin.h"
#include "component_properties.h"
#include "workspace.h"
#include "recent.h"
#include "resources.h"
#include "types.h"
#include "globals.h"
#include "settings.h"
#include "about.h"


class QmWnd : public QMainWindow
{ Q_OBJECT
public:
	using QMainWindow::QMainWindow;

	QmWnd(QWidget* pParent = nullptr);
	virtual ~QmWnd();

	QmWnd(const QmWnd&) = delete;
	const QmWnd& operator=(const QmWnd&) = delete;

	void RestoreSettings();
	void SaveSettings();

	void SetupGUI();
	void SetStatusMessage(const QString& msg);

	void LoadPlugins();

	void Clear();
	void FileNew();
	bool FileLoad();
	bool FileSave();
	bool FileSaveAs();

	bool FileLoadRecent(const QString& filename);

	bool SaveFile(const QString& filename) const;
	bool LoadFile(const QString& filename);

	void ShowSettings(bool only_create = false);
	void ShowAbout();

	Resources& GetResources() { return m_res; }
	const Resources& GetResources() const { return m_res; }


protected:
	virtual void closeEvent(QCloseEvent *) override;

	virtual void dragEnterEvent(QDragEnterEvent *) override;
	virtual void dropEvent(QDropEvent *) override;

	void SetActiveFile();
	bool AskUnsaved();
	QString GetDocDir();

	bool IsWindowModified() const { return m_window_modified; }
	void SetWindowModified(bool b) { m_window_modified = b; }


private:
	QString m_gui_theme{};
	bool m_gui_native{false};

	bool m_window_modified{false};

	QByteArray m_default_window_state{};
	QByteArray m_saved_window_state{};
	QByteArray m_saved_window_geometry{};

	Resources m_res{};
	RecentFiles m_recent{this, 16};

	bool m_auto_calc{true};

	std::shared_ptr<QmScene> m_scene{};
	std::shared_ptr<QmView> m_view{};
	std::shared_ptr<QLabel> m_statusLabel{};
	std::shared_ptr<DockWidgetWrapper<ComponentProperties>> m_properties{};

	std::shared_ptr<Settings> m_settings{};
	std::shared_ptr<About> m_about{};

	ComponentsPlugin m_plugins{};


protected slots:
	void ApplySettings();
	void WorkspaceChanged(bool changed = true);
	void CalculateAllCircuits();
};


#endif
