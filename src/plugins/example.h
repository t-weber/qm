/**
 * example plug-in component
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Dec-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_PLUGIN_EXAMPLE_H__
#define __QM_PLUGIN_EXAMPLE_H__

#include "gui/component_interface.h"


/**
 * identity gate doing nothing
 */
class IdentityGate : public QuantumComponentItem
{
public:
	IdentityGate();
	virtual ~IdentityGate();

	virtual QuantumComponentItem* clone() const override;

	virtual t_uint GetNumQBits() const override { return IdentityGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "identity"; }
	static const char* GetStaticName() { return "Identity Gate"; }

	virtual std::string GetIdent() const override { return IdentityGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return IdentityGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return IdentityGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return true; }

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
};


#endif
