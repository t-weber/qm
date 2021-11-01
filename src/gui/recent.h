/**
 * recent files
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license: see 'LICENSE' file
 */

#ifndef __QM_RECENT_H__
#define __QM_RECENT_H__

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QDir>

#include <memory>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#else
	#include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;
#endif


/**
 * maintains a list of recently opened files
 */
class RecentFiles
{
public:
	RecentFiles(QWidget* parent=nullptr, std::size_t cap=16)
		: m_parentWidget{parent}, m_recent_file_capacity{cap}
	{}

	~RecentFiles() = default;

	// copy constructor
	RecentFiles(const RecentFiles& other) = default;
	RecentFiles& operator=(const RecentFiles& other) = default;


	void SetRecentMenu(std::shared_ptr<QMenu> recent) { m_menuRecent = recent; }
	std::shared_ptr<QMenu> GetRecentMenu() { return m_menuRecent; }

	const QStringList& GetRecentFiles() const { return m_recentFiles; }
	void SetRecentFiles(const QStringList& recent) { m_recentFiles = recent; }

	const QString& GetRecentDir() const { return m_recentDir; }
	void SetRecentDir(const QString& recent) { m_recentDir = recent; }

	const QString& GetOpenFile() const { return m_openFile; }
	void SetOpenFile(const QString& file) { m_openFile = file; }


	/**
	 * create a menu with the recent files
	 */
	template<class t_loadfunc>
	void CreateRecentFileMenu(t_loadfunc loadfunc)
	{
		m_menuRecent->clear();

		for(auto iter = m_recentFiles.begin(); iter != m_recentFiles.end();)
		{
			const QString& filename = *iter;

			fs::path file{filename.toStdString()};
			if(!fs::exists(file))
			{
				iter = m_recentFiles.erase(iter);
				continue;
			}

			QAction *actionFile = new QAction{file.filename().string().c_str(), m_parentWidget};
			actionFile->setToolTip(filename);

			QObject::connect(actionFile, &QAction::triggered, [this, loadfunc, filename]()
			{
				if(loadfunc(filename))
					m_openFile = filename;
			});

			m_menuRecent->addAction(actionFile);

			++iter;
		}
	}


	/**
	 * add a recent file to the list
	 */
	template<class t_loadfunc>
	void AddRecentFile(const QString& filename, t_loadfunc loadfunc)
	{
		m_recentFiles.push_front(filename);
		m_recentFiles.removeDuplicates();

		if((std::size_t)m_recentFiles.size() > m_recent_file_capacity)
			m_recentFiles.erase(m_recentFiles.begin()+m_recent_file_capacity, m_recentFiles.end());

		CreateRecentFileMenu<t_loadfunc>(loadfunc);
	}


private:
	// recent directory
	QString m_recentDir{QDir::homePath()};

	// recent files
	QStringList m_recentFiles{};
	std::shared_ptr<QMenu> m_menuRecent{};

	// currently open file
	QString m_openFile{};

	// parent widget
	QWidget* m_parentWidget{nullptr};

	// maximum number of recent files in the list
	std::size_t m_recent_file_capacity{16};
};


#endif
