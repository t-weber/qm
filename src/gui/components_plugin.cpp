/**
 * plug-in circuit components loader
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Dec-2021
 * @license see 'LICENSE' file
 */


#include <boost/system/error_code.hpp>
namespace sys = boost::system;

#include <boost/dll/shared_library.hpp>
namespace dll = boost::dll;
namespace fs = dll::fs;

#include <iterator>
#include <functional>
#include <vector>
#include <string>
#include <iostream>

#include "components_plugin.h"


static inline std::vector<fs::path> find_plugins(const fs::path& path)
{
	std::vector<fs::path> plugins;

	try
	{
		fs::directory_iterator dir{path};
		for(auto iter=fs::begin(dir); iter!=fs::end(dir); std::advance(iter, 1))
		{
			fs::path plugin = *iter;
			if(plugin.extension() == ".so" || plugin.extension() == ".dylib" || plugin.extension() == ".dll")
				plugins.emplace_back(std::move(plugin));
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
	}

	return plugins;
}


ComponentsPlugin::~ComponentsPlugin()
{
	UnloadPlugins();
}


void ComponentsPlugin::UnloadPlugins()
{
	for(const PluginDescriptor& plugindescr : m_plugins)
	{
		if(!plugindescr.lib)
			continue;

		plugindescr.lib->unload();
	}
}


/**
 * find all plugins in the given path and load them
 */
void ComponentsPlugin::LoadPlugins(const std::string& _path)
{
	fs::path path{_path};
	std::vector<fs::path> possible_plugins = find_plugins(path);

	for(const fs::path& possible_plugin : possible_plugins)
	{
		if(!fs::exists(possible_plugin))
			continue;

		try
		{
			PluginDescriptor plugin{};

			// see: https://www.boost.org/doc/libs/1_78_0/doc/html/boost/dll/shared_library.html
			// see: https://www.boost.org/doc/libs/1_78_0/doc/html/boost/dll/load_mode/type.html
			plugin.lib = std::make_shared<dll::shared_library>();

			sys::error_code err{};
			plugin.lib->load(possible_plugin, err,
				//dll::load_mode::default_mode |
				dll::load_mode::rtld_lazy |
				dll::load_mode::rtld_local |
				//dll::load_mode::rtld_deepbind |
				dll::load_mode::append_decorations);

			if(!plugin.lib->is_loaded() || err)
			{
				std::cerr << "Could not load \"" << fs::absolute(possible_plugin).string()
					<< "\": " << err.message() << "." << std::endl;
				continue;
			}

			// get api functions
			const char* get_comp_descr_name = QM_ADD_QUOTES(QM_PLUGIN_FUNC_GET_COMP_DESCR);
			const char* get_create_comp_name = QM_ADD_QUOTES(QM_PLUGIN_FUNC_CREATE_COMP);

			if(plugin.lib->has(get_comp_descr_name))
			{
				plugin.func_get_comp_descr =
					plugin.lib->get<t_plugin_func_get_comp_descr>(get_comp_descr_name);
			}
			if(plugin.lib->has(get_create_comp_name))
			{
				plugin.create_comp =
					plugin.lib->get<t_plugin_func_create_comp>(get_create_comp_name);
			}

			if(!plugin.func_get_comp_descr || !plugin.create_comp)
			{
				std::cerr << "Required API functions could not be found in plugin "
					<< plugin.lib->location() << "." << std::endl;
				plugin.lib->unload();
				continue;
			}

			// get info from the plugin
			plugin.descr = (*plugin.func_get_comp_descr)();

			m_plugins.emplace_back(std::move(plugin));
		}
		catch(const std::exception& ex)
		{
			std::cerr << "Error: " << ex.what() << "." << std::endl;
		}
	}
}


/**
 * the the identifiers and names of all quantum components from all loaded plugins
 */
std::vector<std::tuple<std::string, std::string>> ComponentsPlugin::GetComponentNames() const
{
	std::vector<std::tuple<std::string, std::string>> comp_names;

	for(const PluginDescriptor& plugindescr : m_plugins)
	{
		for(const PluginComponentDescriptor& compdescr : plugindescr.descr)
		{
			comp_names.emplace_back(std::make_tuple(compdescr.ident, compdescr.name));
		}
	}

	return comp_names;
}


/**
 * create the quantum component with the given identifier
 */
QuantumComponentItem* ComponentsPlugin::CreateComponent(const std::string& ident) const
{
	// look for the plugin which handles the component identifier
	for(const PluginDescriptor& plugindescr : m_plugins)
	{
		if(!plugindescr.create_comp)
			continue;

		if(QuantumComponentItem* item = (*plugindescr.create_comp)(ident); item)
			return item;
	}

	return nullptr;
}
