/**
 * circuit component properties
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_properties.h"
#include "settings.h"

#include <QtWidgets/QGridLayout>


ComponentProperties::ComponentProperties(QWidget* parent)
	: QWidget(parent)
{
	auto* layout = new QGridLayout(this);
	layout->setContentsMargins(4, 4, 4, 4);
	layout->setVerticalSpacing(4);
	layout->setHorizontalSpacing(4);
}


ComponentProperties::~ComponentProperties()
{
}
