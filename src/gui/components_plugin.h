/**
 * plug-in circuit components loader
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Dec-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENTS_PLUGIN_H__
#define __QM_COMPONENTS_PLUGIN_H__


#include <boost/dll/shared_library.hpp>

#include <string>
#include <memory>
#include <vector>
#include <tuple>

#include "component_interface.h"


/**
 * struct describing one plugin
 */
struct PluginDescriptor
{
	// plugin library
	std::shared_ptr<boost::dll::shared_library> lib{};

	// api functions
	t_plugin_func_setup setup{nullptr};
	t_plugin_func_get_comp_descr get_comp_descr{nullptr};
	t_plugin_func_create_comp create_comp{nullptr};

	// descriptors for the actual plugin components
	std::vector<PluginComponentDescriptor> descr{};
};


/**
 * plugin manager
 */
class ComponentsPlugin
{
public:
	ComponentsPlugin() = default;
	~ComponentsPlugin();

	void LoadPlugins(const std::string& path);
	void UnloadPlugins();

	const std::vector<PluginDescriptor>& GetDescriptors() const { return m_plugins; }

	std::vector<std::tuple<std::string, std::string>> GetComponentNames() const;
	QuantumComponentItem* CreateComponent(const std::string& ident) const;


private:
	std::vector<PluginDescriptor> m_plugins{};
};


#endif
