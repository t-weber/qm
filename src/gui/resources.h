/**
 * resource files
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_RESOURCE_H__
#define __QM_RESOURCE_H__

#include <vector>
#include <optional>

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


class Resources
{
public:
	Resources() = default;
	~Resources() = default;

	void AddPath(const std::string& path);
	void AddPath(const fs::path& path);

	std::optional<fs::path> FindFile(const std::string& file) const;
	std::optional<fs::path> FindFile(const fs::path& file) const;


private:
	std::vector<fs::path> m_paths{};
};

#endif
