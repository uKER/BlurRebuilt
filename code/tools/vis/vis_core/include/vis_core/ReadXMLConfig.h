#ifndef __READ_XML_CONFIG_H__
#define __READ_XML_CONFIG_H__

#include <btl/Containers/vector.h>
#include "vis_core/LightingPass.h"

class VisConfig
{
public:
	bool Read(const std::string& fileName);

	~VisConfig();

	Btl::Vector<LightingPass*> m_Passes;

private:
	// read xml data
	void VisConfig::ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode);
};

#endif
