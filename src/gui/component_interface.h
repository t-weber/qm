/**
 * circuit component interface
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENT_IF_H__
#define __QM_COMPONENT_IF_H__

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QPainter>

#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <tuple>

#include "types.h"
#include "helpers.h"


// ----------------------------------------------------------------------------
// configuration structs
// ----------------------------------------------------------------------------
/**
 * component configuration options
 */
struct ComponentConfig
{
	// key
	std::string key{};

	// value
	std::variant<
		t_int, t_uint,
		t_real, t_cplx,
		std::string> value{};

	// description of the entry
	std::string description{};

	// optional minimum and maximum values
	std::optional<std::variant<
		t_int, t_uint,
		t_real, std::string>> min_value{};
	std::optional<std::variant<
		t_int, t_uint,
		t_real, std::string>> max_value{};

	bool is_phase{false};
};


/**
 * set the configuration of a qubit
 */
struct QBitConfig
{
	t_uint bit{};

	// 0:down, 1:up
	t_uint component{};

	// component value
	t_real value{};
};


/**
 * struct for the exchange of component configuration options
 */
struct ComponentConfigs
{
	std::string name{};

	std::vector<ComponentConfig> configs{};
	std::vector<QBitConfig> qbit_configs{};
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// interface classes for quantum circuit components
// ----------------------------------------------------------------------------
enum class ComponentType
{
	STATE,
	GATE
};


/**
 * basic interface for a quantum component:
 * either a collection of states or a gate
 */
class QuantumComponent
{
public:
	virtual ~QuantumComponent() = default;

	virtual std::string GetIdent() const = 0;
	virtual std::string GetName() const = 0;

	virtual ComponentType GetType() const = 0;
	virtual t_mat GetOperator() const = 0;
	virtual bool IsOk() const = 0;

	virtual ComponentConfigs GetConfig() const = 0;
	virtual void SetConfig(const ComponentConfigs&) = 0;

	virtual t_uint GetNumQBits() const = 0;
};


/**
 * a graphical representation of a quantum component
 */
class QuantumComponentItem : public QuantumComponent, public QGraphicsItem
{
public:
	QuantumComponentItem() = default;
	virtual ~QuantumComponentItem() = default;

	virtual QuantumComponentItem* clone() const = 0;

	// factory function
	static QuantumComponentItem* create(const std::string& id);

	std::tuple<t_int, t_int> GetGridPos() const
	{
		return get_grid_indices(scenePos(), g_raster_size, g_raster_size);
	}

	void SetGridPos(t_int x, t_int y)
	{
		QPointF posScene(x*g_raster_size, y*g_raster_size);
		this->setPos(posScene);
	}
};


// shared pointer to a QuantumComponentItem
using t_gateptr = std::shared_ptr<QuantumComponentItem>;
// [column index, column operator]
using t_columnop = std::tuple<bool, std::size_t, t_mat>;
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// plugin interface
// ----------------------------------------------------------------------------
struct PluginComponentDescriptor
{
	std::string ident{};
	std::string name{};

	ComponentType ty{ComponentType::GATE};
	t_uint min_qbits{0};
};

struct PluginSettings
{
	t_real *raster_size = &::g_raster_size;
};

// plugin api functions
using t_plugin_func_setup = void(*)(const PluginSettings&);
using t_plugin_func_get_comp_descr = std::vector<PluginComponentDescriptor>(*)();
using t_plugin_func_create_comp = QuantumComponentItem*(*)(const std::string& ident);

// plugin api function names
#define QM_PLUGIN_FUNC_SETUP           setup
#define QM_PLUGIN_FUNC_GET_COMP_DESCR  get_component_descriptors
#define QM_PLUGIN_FUNC_CREATE_COMP     create_component

#define QM_ADD_QUOTES2(macro) #macro
#define QM_ADD_QUOTES(macro)  QM_ADD_QUOTES2(macro)
// ----------------------------------------------------------------------------


#endif
