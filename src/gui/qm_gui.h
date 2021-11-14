/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_GUI_H__
#define __QM_GUI_H__

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>

#include <memory>
#include <vector>

#include "components.h"
#include "component_properties.h"
#include "workspace.h"
#include "recent.h"
#include "resources.h"
#include "types.h"
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

	void SetupGUI();
	void SetStatusMessage(const QString& msg);

	void Clear();
	void FileNew();
	void FileLoad();
	void FileSave();
	void FileSaveAs();

	bool FileLoadRecent(const QString& filename);

	bool SaveFile(const QString& filename) const;
	bool LoadFile(const QString& filename);

	void ShowSettings();
	void ShowAbout();

	Resources& GetResources() { return m_res; }
	const Resources& GetResources() const { return m_res; }


protected:
	virtual void closeEvent(QCloseEvent *) override;

	virtual void dragEnterEvent(QDragEnterEvent *) override;
	virtual void dropEvent(QDropEvent *) override;


private:
	QString m_gui_theme{};
	bool m_gui_native{false};

	Resources m_res{};
	RecentFiles m_recent{this, 16};

	std::shared_ptr<QmScene> m_scene{};
	std::shared_ptr<QmView> m_view{};
	std::shared_ptr<QLabel> m_statusLabel{};
	std::shared_ptr<DockWidgetWrapper<ComponentProperties>> m_properties{};

	std::shared_ptr<Settings> m_settings{};
	std::shared_ptr<About> m_about{};
};


#endif
